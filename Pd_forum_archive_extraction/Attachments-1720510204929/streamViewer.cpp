#include <sys/time.h>
#include <pthread.h>
#include <gtk/gtk.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif
#include <gst/app/gstappsrc.h>
#include <gst/interfaces/xoverlay.h>
#include <m3api/xiApi.h>
#include <m3api/xiExt.h>

BOOL acquire, quitting, render = TRUE;
int maxcx, maxcy, roix0, roiy0, roicx, roicy;
pthread_t videoThread;
HANDLE handle = INVALID_HANDLE_VALUE;
guintptr window_handle;

GstBusSyncReply bus_sync_handler(GstBus* bus, GstMessage* message, gpointer) {
	if(GST_MESSAGE_TYPE(message) != GST_MESSAGE_ELEMENT || !gst_structure_has_name(message->structure, "prepare-xwindow-id"))
		return GST_BUS_PASS;
#if GST_CHECK_VERSION(0,10,31)
	gst_x_overlay_set_window_handle(GST_X_OVERLAY(GST_MESSAGE_SRC(message)), window_handle);
#else
	gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(GST_MESSAGE_SRC(message)), window_handle);
#endif
	gst_message_unref(message);
	return GST_BUS_DROP;
}

#if defined(GDK_WINDOWING_X11)
void video_widget_realize_cb(GtkWidget* widget, gpointer) {
	GdkWindow *window = gtk_widget_get_window(widget);
	if (!gdk_window_ensure_native(window))
		g_error ("Couldn't create native window needed for GstXOverlay!");
	window_handle = GDK_WINDOW_XID(window);
}
#elif defined(GDK_WINDOWING_QUARTZ)
void video_widget_realize_cb(GtkWidget*, gpointer);
#else
#error Unsupported GDK backend
#endif

inline unsigned long getcurus() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000000 + now.tv_usec;
}

gboolean close_cb(GtkWidget*, GdkEvent*, gpointer quit) {
	quitting = quit ? TRUE : FALSE;
	if(videoThread)
		acquire = FALSE;
	else
		gtk_main_quit();
	return TRUE;
}

void* videoDisplay(void*) {
	GtkWidget *videoWindow;
	GdkScreen *screen;
	GstElement *pipeline, *appsrc/*, *fpssink*/;
	GstFlowReturn ret;
	GstBuffer *buffer;
	GstBus *bus;
	GstCaps *caps = 0;
	int max_width, max_height;
	int prev_width = -1;
	int prev_height = -1;
	unsigned long frames = 0;
	unsigned long prevframes = 0;
	unsigned long lostframes = 0;
	unsigned long curtime, prevtime;
	long lastframe = -1;
	gchar/* *videofps,*/ title[256];
	XI_IMG_FORMAT prev_format = XI_RAW8;
	XI_IMG image;
	image.size = sizeof(XI_IMG);
	image.bp = NULL;
	image.bp_size = 0;
	gdk_threads_enter();
	videoWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(videoWindow), "streamViewer");
	gtk_widget_set_double_buffered(videoWindow, FALSE);
	g_signal_connect(videoWindow, "realize", G_CALLBACK(video_widget_realize_cb), NULL);
	g_signal_connect(videoWindow, "delete-event", G_CALLBACK(close_cb), NULL);
	gtk_widget_show_all(videoWindow);
	gtk_widget_realize(videoWindow);
	screen = gdk_screen_get_default();
	max_width = 0.8*gdk_screen_get_width(screen);
	max_height = 0.8*gdk_screen_get_width(screen);
	gdk_threads_leave();
	if(xiStartAcquisition(handle) != XI_OK)
		goto exit;
	//pipeline = gst_parse_launch("appsrc is-live=TRUE name=streamViewer ! queue max-size-buffers=2 leaky=2 ! ffmpegcolorspace ! videoscale add-borders=TRUE ! fpsdisplaysink name=fpssink text-overlay=FALSE sync=FALSE", NULL);
	pipeline = gst_parse_launch("appsrc is-live=TRUE name=streamViewer ! queue max-size-buffers=2 leaky=2 ! ffmpegcolorspace ! videoscale add-borders=TRUE ! autovideosink", NULL);
	if(!pipeline)
		goto exit;
	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	gst_bus_set_sync_handler(bus, (GstBusSyncHandler)bus_sync_handler, NULL);
	gst_object_unref(bus);
	appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "streamViewer");
	//fpssink = gst_bin_get_by_name(GST_BIN(pipeline), "fpssink");
	gst_element_set_state(pipeline, GST_STATE_PLAYING);
	prevtime = getcurus();
	while(acquire) {
		if(xiGetImage(handle, 5000, &image) != XI_OK)
			break;
		if(render) {
			buffer = gst_buffer_new();
			gst_buffer_set_data(buffer, (guint8*)image.bp, image.width*image.height*(image.frm == XI_RAW8 ? 1 : 3));
			GST_BUFFER_TIMESTAMP(buffer) = GST_CLOCK_TIME_NONE;
			if(prev_width != image.width || prev_height != image.height || prev_format != image.frm) {
				if(caps)
					gst_caps_unref(caps);
				if(image.frm == XI_RAW8)
					caps = gst_caps_new_simple ("video/x-raw-gray",
							"bpp", G_TYPE_INT, 8,
							"depth", G_TYPE_INT, 8,
							"framerate", GST_TYPE_FRACTION, 0, 1,
							"width", G_TYPE_INT, image.width,
							"height", G_TYPE_INT, image.height,
							NULL);
				else if(image.frm == XI_RGB24)
					caps = gst_caps_new_simple ("video/x-raw-rgb",
							"bpp", G_TYPE_INT, 24,
							"depth", G_TYPE_INT, 24,
							"endianness", G_TYPE_INT, G_BIG_ENDIAN,
							"red_mask",   G_TYPE_INT, 0x0000ff,
							"green_mask", G_TYPE_INT, 0x00ff00,
							"blue_mask",  G_TYPE_INT, 0xff0000,
							"framerate", GST_TYPE_FRACTION, 0, 1,
							"width", G_TYPE_INT, image.width,
							"height", G_TYPE_INT, image.height,
							NULL);
				else
					break;
				gst_buffer_set_caps(buffer, caps);
				prev_width = image.width;
				prev_height = image.height;
				prev_format = image.frm;
				int width = image.width;
				int height = image.height;
				while(width > max_width || height > max_height) {
					width /= 2;
					height /=2;
				}
				gdk_threads_enter();
				gtk_window_resize(GTK_WINDOW(videoWindow), width, height);
				gdk_threads_leave();
			}
			ret = gst_app_src_push_buffer(GST_APP_SRC(appsrc), buffer);
			if(ret != GST_FLOW_OK)
				break;
		}
		frames++;
		if(image.nframe > lastframe)
			lostframes += image.nframe - (lastframe + 1);
		lastframe = image.nframe;
		curtime = getcurus();
		if(curtime - prevtime > 1000000) {
			//g_object_get(G_OBJECT(fpssink), "last-message", &videofps, NULL);
			//snprintf(title, 256, "Acquisition [ captured: %lu, skipped: %lu, fps: %.2f ]; Video [ %s ]", frames, lostframes, 1000000.0 * (frames - prevframes) / (curtime - prevtime), videofps);
			snprintf(title, 256, "Acquisition [ captured: %lu, skipped: %lu, fps: %.2f ]", frames, lostframes, 1000000.0 * (frames - prevframes) / (curtime - prevtime));
			gtk_window_set_title(GTK_WINDOW(videoWindow), title);
			//g_free(videofps);
			prevframes = frames;
			prevtime = curtime;
		}
	}
	if(caps)
		gst_caps_unref(caps);
	gst_app_src_end_of_stream(GST_APP_SRC(appsrc));
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(pipeline));
exit:
	gdk_threads_enter();
	gtk_widget_destroy(videoWindow);
	if(quitting)
		gtk_main_quit();
	xiStopAcquisition(handle);
	acquire = FALSE;
	xiCloseDevice(handle);
	handle = INVALID_HANDLE_VALUE;
	videoThread = 0;
	gdk_threads_leave();
	return 0;
}

struct ctrl_window {
    GtkWidget *window, *boxmain, *boxgpi, *boxx, *boxy, *gpi1, *gpi2, *gpi3, *gpi4, *labelexp, *exp, *labelgain, *gain, *labelx0, *x0, *labely0, *y0, *labelcx, *cx, *labelcy, *cy, *raw, *show, *run;
};

gboolean time_handler(ctrl_window *ctrl) {
	int level = 0;
	if(acquire && handle != INVALID_HANDLE_VALUE) {
		xiSetParamInt(handle, XI_PRM_GPI_SELECTOR, 1);
		xiGetParamInt(handle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl->gpi1), level);
		xiSetParamInt(handle, XI_PRM_GPI_SELECTOR, 2);
		xiGetParamInt(handle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl->gpi2), level);
		xiSetParamInt(handle, XI_PRM_GPI_SELECTOR, 3);
		xiGetParamInt(handle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl->gpi3), level);
		xiSetParamInt(handle, XI_PRM_GPI_SELECTOR, 4);
		xiGetParamInt(handle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl->gpi4), level);
	}
	gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(ctrl->run), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctrl->run)) != acquire);
	gtk_widget_set_sensitive(ctrl->boxx, acquire);
	gtk_widget_set_sensitive(ctrl->boxy, acquire);
	gtk_widget_set_sensitive(ctrl->exp, acquire);
	gtk_widget_set_sensitive(ctrl->gain, acquire);
	return TRUE;
}

gboolean update_x0(GtkAdjustment *adj, gpointer) {
	roix0 = gtk_adjustment_get_value(adj);
	if(roicx + roix0 > maxcx){
		roix0 = maxcx - roicx;
		gtk_adjustment_set_value(adj, roix0);
	}
	xiSetParamInt(handle, XI_PRM_OFFSET_X, roix0);
	return TRUE;
}

gboolean update_y0(GtkAdjustment *adj, gpointer) {
	roiy0 = gtk_adjustment_get_value(adj);
	if(roicy + roiy0 > maxcy){
		roiy0 = maxcy - roicy;
		gtk_adjustment_set_value(adj, roiy0);
	}
	xiSetParamInt(handle, XI_PRM_OFFSET_Y, roiy0);
	return TRUE;
}

gboolean update_cx(GtkAdjustment *adj, gpointer) {
	roicx = gtk_adjustment_get_value(adj);
	if(roix0 + roicx > maxcx){
		roicx = maxcx - roix0;
		gtk_adjustment_set_value(adj, roicx);
	}
	xiSetParamInt(handle, XI_PRM_WIDTH, roicx);
	return TRUE;
}

gboolean update_cy(GtkAdjustment *adj, gpointer) {
	roicy = gtk_adjustment_get_value(adj);
	if(roiy0 + roicy > maxcy) {
		roicy = maxcy - roiy0;
		gtk_adjustment_set_value(adj, roicy);
	}
	xiSetParamInt(handle, XI_PRM_HEIGHT, roicy);
	return TRUE;
}

gboolean update_exposure(GtkAdjustment *adj, gpointer) {
	xiSetParamInt(handle, XI_PRM_EXPOSURE, 1000*gtk_adjustment_get_value(adj));		
	return TRUE;
}

gboolean update_gain(GtkAdjustment *adj, gpointer) {
	xiSetParamFloat(handle, XI_PRM_GAIN, gtk_adjustment_get_value(adj));
	return TRUE;
}

gboolean update_raw(GtkToggleButton *raw, ctrl_window *ctrl) {
	if(handle != INVALID_HANDLE_VALUE) {
		float mingain, maxgain;
		xiSetParamInt(handle, XI_PRM_IMAGE_DATA_FORMAT, gtk_toggle_button_get_active(raw) ? XI_RAW8 : XI_RGB24);
		xiGetParamFloat(handle, XI_PRM_GAIN XI_PRM_INFO_MIN, &mingain);
		xiGetParamFloat(handle, XI_PRM_GAIN XI_PRM_INFO_MAX, &maxgain);
		xiGetParamInt(handle, XI_PRM_WIDTH XI_PRM_INFO_MAX, &maxcx);
		xiGetParamInt(handle, XI_PRM_HEIGHT XI_PRM_INFO_MAX, &maxcy);
		roicx = maxcx;
		roicy = maxcy;
		roix0 = 0;
		roiy0 = 0;
		gtk_adjustment_configure(gtk_range_get_adjustment(GTK_RANGE(ctrl->gain)), mingain, mingain, maxgain, 0.1, 1, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl->x0)), roix0, 0, maxcx-4, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl->y0)), roiy0, 0, maxcy-2, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl->cx)), roicx, 4, maxcx, 4, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl->cy)), roicy, 2, maxcy, 2, 20, 0);
		xiSetParamFloat(handle, XI_PRM_GAIN, mingain);
		xiSetParamInt(handle, XI_PRM_OFFSET_X, roix0);
		xiSetParamInt(handle, XI_PRM_OFFSET_Y, roiy0);
		xiSetParamInt(handle, XI_PRM_WIDTH, roicx);
		xiSetParamInt(handle, XI_PRM_HEIGHT, roicy);
		//exposure doesn't seem to be affected by format change
	}
	return TRUE;
}

gboolean update_show(GtkToggleButton *show, gpointer) {
	render = gtk_toggle_button_get_active(show);
	return TRUE;
}

gboolean update_run(GtkToggleButton *run, ctrl_window *ctrl) {
	gtk_toggle_button_set_inconsistent(run, false);
	acquire = gtk_toggle_button_get_active(run);
	if(acquire && handle == INVALID_HANDLE_VALUE) {
		DWORD nIndex = 0;
		char* env = getenv("CAM_INDEX");
		if(env) {
			nIndex = atoi(env);
		}
		DWORD tmp;
		xiGetNumberDevices(&tmp); //rescan available devices
		if(xiOpenDevice(nIndex, &handle) != XI_OK) {
			printf("Couldn't setup camera!\n");
			acquire = FALSE;
			return TRUE;
		}
		update_raw(GTK_TOGGLE_BUTTON(ctrl->raw), ctrl);
		xiSetParamInt(handle, XI_PRM_AUTO_WB, 1);
		xiSetParamInt(handle, XI_PRM_EXPOSURE, 10000);
		gtk_adjustment_set_value(gtk_range_get_adjustment(GTK_RANGE(ctrl->exp)), 10);
		if(pthread_create(&videoThread, NULL, videoDisplay, NULL))
			exit(1);
	}
	gtk_widget_set_sensitive(ctrl->boxx, acquire);
	gtk_widget_set_sensitive(ctrl->boxy, acquire);
	gtk_widget_set_sensitive(ctrl->exp, acquire);
	gtk_widget_set_sensitive(ctrl->gain, acquire);
	return TRUE;
}

gboolean start_cb(ctrl_window* ctrl) {
	//start acquisition
	update_run(GTK_TOGGLE_BUTTON(ctrl->run), ctrl);
	return FALSE;
}

int main(int argc, char **argv)
{
	ctrl_window ctrl;
	g_thread_init(NULL);
	gdk_threads_init();
	gdk_threads_enter();
	gst_init(&argc, &argv);
	gtk_init(&argc, &argv);
	//create widgets
	ctrl.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	ctrl.boxmain = gtk_vbox_new(FALSE, 0);
	ctrl.boxgpi = gtk_hbox_new(TRUE, 0);
	ctrl.boxx = gtk_hbox_new(FALSE, 0);
	ctrl.boxy = gtk_hbox_new(FALSE, 0);
	ctrl.labelexp = gtk_label_new("Exposure (ms)");
	ctrl.labelgain = gtk_label_new("Gain (dB)");
	ctrl.labelx0 = gtk_label_new("x0");
	ctrl.labelcx = gtk_label_new("cx");
	ctrl.labely0 = gtk_label_new("y0");
	ctrl.labelcy = gtk_label_new("cy");
	ctrl.gpi1 = gtk_check_button_new_with_label("GPI1");
	ctrl.gpi2 = gtk_check_button_new_with_label("GPI2");
	ctrl.gpi3 = gtk_check_button_new_with_label("GPI3");
	ctrl.gpi4 = gtk_check_button_new_with_label("GPI4");
	ctrl.exp = gtk_hscale_new_with_range(1, 1000, 1);
	ctrl.gain = gtk_hscale_new_with_range(0, 1, 0.1); //use dummy limits
	ctrl.x0 = gtk_spin_button_new_with_range(0, 128, 2); //use dummy max limit
	ctrl.y0 = gtk_spin_button_new_with_range(0, 128, 2); //use dummy max limit
	ctrl.cx = gtk_spin_button_new_with_range(4, 128, 4); //use dummy max limit
	ctrl.cy = gtk_spin_button_new_with_range(2, 128, 2); //use dummy max limit
	ctrl.raw = gtk_toggle_button_new_with_label("Display RAW data");
	ctrl.show = gtk_toggle_button_new_with_label("Live view");
	ctrl.run = gtk_toggle_button_new_with_label("Acquisition");
	//tune them
	gtk_window_set_title(GTK_WINDOW(ctrl.window), "streamViewer control");
	gtk_window_set_keep_above(GTK_WINDOW(ctrl.window), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl.raw), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl.show), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl.run), TRUE); //actual start is delayed by 100ms, see below
	gtk_widget_set_sensitive(ctrl.boxgpi, FALSE);
	gtk_scale_set_digits(GTK_SCALE(ctrl.exp), 0);
	gtk_range_set_update_policy(GTK_RANGE(ctrl.exp), GTK_UPDATE_DISCONTINUOUS);
	gtk_range_set_update_policy(GTK_RANGE(ctrl.gain), GTK_UPDATE_DISCONTINUOUS);
	gtk_adjustment_set_value(gtk_range_get_adjustment(GTK_RANGE(ctrl.exp)), 10);
	gtk_scale_set_value_pos(GTK_SCALE(ctrl.exp), GTK_POS_RIGHT);
	gtk_scale_set_value_pos(GTK_SCALE(ctrl.gain), GTK_POS_RIGHT);
	gtk_widget_set_sensitive(ctrl.boxx, FALSE);
	gtk_widget_set_sensitive(ctrl.boxy, FALSE);
	gtk_widget_set_sensitive(ctrl.exp, FALSE);
	gtk_widget_set_sensitive(ctrl.gain, FALSE);
	//pack everything into window
	gtk_container_add(GTK_CONTAINER(ctrl.boxgpi), ctrl.gpi1);
	gtk_container_add(GTK_CONTAINER(ctrl.boxgpi), ctrl.gpi2);
	gtk_container_add(GTK_CONTAINER(ctrl.boxgpi), ctrl.gpi3);
	gtk_container_add(GTK_CONTAINER(ctrl.boxgpi), ctrl.gpi4);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.boxgpi);
	gtk_container_add(GTK_CONTAINER(ctrl.boxx), ctrl.labelx0);
	gtk_container_add(GTK_CONTAINER(ctrl.boxx), ctrl.x0);
	gtk_container_add(GTK_CONTAINER(ctrl.boxy), ctrl.labely0);
	gtk_container_add(GTK_CONTAINER(ctrl.boxy), ctrl.y0);
	gtk_container_add(GTK_CONTAINER(ctrl.boxx), ctrl.labelcx);
	gtk_container_add(GTK_CONTAINER(ctrl.boxx), ctrl.cx);
	gtk_container_add(GTK_CONTAINER(ctrl.boxy), ctrl.labelcy);
	gtk_container_add(GTK_CONTAINER(ctrl.boxy), ctrl.cy);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.boxx);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.boxy);	
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.labelexp);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.exp);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.labelgain);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.gain);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.raw);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.show);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.run);
	gtk_container_add(GTK_CONTAINER(ctrl.window), ctrl.boxmain);
	//register handlers
	gdk_threads_add_timeout(1000, (GSourceFunc)time_handler, (gpointer)&ctrl);
	gdk_threads_add_timeout(100, (GSourceFunc)start_cb, (gpointer)&ctrl); //only way I found to make sure window is displayed right away
	g_signal_connect(ctrl.window, "delete_event", G_CALLBACK(close_cb), (gpointer)TRUE);
	g_signal_connect(gtk_range_get_adjustment(GTK_RANGE(ctrl.gain)), "value_changed", G_CALLBACK(update_gain), NULL);
	g_signal_connect(gtk_range_get_adjustment(GTK_RANGE(ctrl.exp)), "value_changed", G_CALLBACK(update_exposure), NULL);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl.x0)), "value_changed", G_CALLBACK(update_x0), NULL);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl.y0)), "value_changed", G_CALLBACK(update_y0), NULL);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl.cx)), "value_changed", G_CALLBACK(update_cx), NULL);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl.cy)), "value_changed", G_CALLBACK(update_cy), NULL);
	g_signal_connect(ctrl.raw, "toggled", G_CALLBACK(update_raw), (gpointer)&ctrl);
	g_signal_connect(ctrl.show, "toggled", G_CALLBACK(update_show), NULL);
	g_signal_connect(ctrl.run, "toggled", G_CALLBACK(update_run), (gpointer)&ctrl);
	//show window
	gtk_widget_show_all(ctrl.window);
	//start the main loop
	gtk_main();
	//exit
	gdk_threads_leave();
	acquire = FALSE;
	if(videoThread)
		pthread_join(videoThread, NULL);
	return 0;
}
