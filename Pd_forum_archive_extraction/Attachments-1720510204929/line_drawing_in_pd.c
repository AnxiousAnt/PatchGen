/* line drawing in pd. These are the functions that draw lines connecting objects in pd 0.39-2 test 7 */
/* lines prefixed with - are to be removed and replaced with the lines starting with + */
/* if you want to use coloured lines */
/* 2006102 Martin Peach */

/* g_canvas.c 894: */
/* canvas_drawlines is called only from canvas_map in g_canvas.c. This draws lines in newly opened canvases */
static void canvas_drawlines(t_canvas *x)
{
    t_linetraverser t;
    t_outconnect *oc;
    {
        linetraverser_start(&t, x);
        while (oc = linetraverser_next(&t))
-            sys_vgui(".x%lx.c create line %d %d %d %d -width %d -tags l%lx\n",
-                    glist_getcanvas(x),
-                        t.tr_lx1, t.tr_ly1, t.tr_lx2, t.tr_ly2,
-                            (outlet_getsymbol(t.tr_outlet) == &s_signal ? 2:1),
-                                oc);
+		{
+			sys_vgui(".x%lx.c create line %d %d %d %d -width %d -tags l%lx -fill #%06lX\n",
+                    glist_getcanvas(x),
+                        t.tr_lx1, t.tr_ly1, t.tr_lx2, t.tr_ly2,
+                            (outlet_getsymbol(t.tr_outlet) == &s_signal ? 2:1),
+                                oc,
+									(((long)oc<<2) & 0x0FFFFFF));
+		}
    }
}



/* g_canvas.c 909: */
/* canvas_fixlinesfor is called by iemgui_size, iemgui_delta, iemgui_pos, iemgui_displace in g_all_guis.c; */
/* by bng_dialog in g_bang.c; */
/* by glist_deselect, canvas_setgraph, canvas_connect(see above) in g_editor.c; */
/* by canvas_addinlet, canvas_rminlet, canvas_resortinlets, canvas_addoutlet, canvas_rmoutlet; */
/* canvas_resortoutlets, graph_displace in g_graph.c */
/* by hradio_dialog, hslider_dialog, my_numbox_dialog, text_displace, */
/* toggle_dialog, vradio_dialog, vslider_dialog, vu_dialog, vu_size */
/* canvas_fixlinesfor sets the coordinates of lines after the objects they connect have changed */
void canvas_fixlinesfor(t_canvas *x, t_text *text)
{
    t_linetraverser t;
    t_outconnect *oc;

    linetraverser_start(&t, x);
    while (oc = linetraverser_next(&t))
    {
        if (t.tr_ob == text || t.tr_ob2 == text)
        {
            sys_vgui(".x%lx.c coords l%lx %d %d %d %d\n",
                glist_getcanvas(x), oc,
                    t.tr_lx1, t.tr_ly1, t.tr_lx2, t.tr_ly2);
+			sys_vgui(".x%lx.c itemconfigure l%lx -fill #%06lX\n",
+				glist_getcanvas(x), oc, (((long)oc<<2) & 0x0FFFFFF));
        }
    }
}

/* g_editor.c 99: */
/* glist_selectline is called only by canvas_doclick in g_editor.c */
/* glist_selectline sets the colour of the selected line to blue */
void glist_selectline(t_glist *x, t_outconnect *oc, int index1,
    int outno, int index2, int inno)
{
    if (x->gl_editor)
    {
        glist_noselect(x);
        x->gl_editor->e_selectedline = 1;
        x->gl_editor->e_selectline_index1 = index1;
        x->gl_editor->e_selectline_outno = outno;
        x->gl_editor->e_selectline_index2 = index2;
        x->gl_editor->e_selectline_inno = inno;
        x->gl_editor->e_selectline_tag = oc;
        sys_vgui(".x%lx.c itemconfigure l%lx -fill blue\n",
            x, x->gl_editor->e_selectline_tag);
    }
}

/* glist_deselectline is called by glist_select and glist_noselect in g_editor.c */
/* glist_deselectline restores the colour of a deselcted line */
void glist_deselectline(t_glist *x)
{
    if (x->gl_editor)
    {
        x->gl_editor->e_selectedline = 0;
-        sys_vgui(".x%lx.c itemconfigure l%lx -fill black\n",
-            x, x->gl_editor->e_selectline_tag);
+        sys_vgui(".x%lx.c itemconfigure l%lx -fill #%06lX\n",
+            x, x->gl_editor->e_selectline_tag,
+				(((long)x->gl_editor->e_selectline_tag<<2) & 0x0FFFFFF));
    }
}

/* g_editor.c 1268: */
/* canvas_doconnect is called only from canvas_mouseup and canvas_motion in g_editor.c */
/* This draws newly connected lines on mouseup (doit = 1) but not motion (doit=0) */
void canvas_doconnect(t_canvas *x, int xpos, int ypos, int which, int doit)
{
    int x11=0, y11=0, x12=0, y12=0;
    t_gobj *y1;
    int x21=0, y21=0, x22=0, y22=0;
    t_gobj *y2;
    int xwas = x->gl_editor->e_xwas,
        ywas = x->gl_editor->e_ywas;
    if (doit) sys_vgui(".x%lx.c delete x\n", x);
    else sys_vgui(".x%lx.c coords x %d %d %d %d\n",
            x, x->gl_editor->e_xwas,
                x->gl_editor->e_ywas, xpos, ypos);

    if ((y1 = canvas_findhitbox(x, xwas, ywas, &x11, &y11, &x12, &y12))
        && (y2 = canvas_findhitbox(x, xpos, ypos, &x21, &y21, &x22, &y22)))
    {
        t_object *ob1 = pd_checkobject(&y1->g_pd);
        t_object *ob2 = pd_checkobject(&y2->g_pd);
        int noutlet1, ninlet2;
        if (ob1 && ob2 && ob1 != ob2 &&
            (noutlet1 = obj_noutlets(ob1))
            && (ninlet2 = obj_ninlets(ob2)))
        {
            int width1 = x12 - x11, closest1, hotspot1;
            int width2 = x22 - x21, closest2, hotspot2;
            int lx1, lx2, ly1, ly2;
            t_outconnect *oc;

            if (noutlet1 > 1)
            {
                closest1 = ((xwas-x11) * (noutlet1-1) + width1/2)/width1;
                hotspot1 = x11 +
                    (width1 - IOWIDTH) * closest1 / (noutlet1-1);
            }
            else closest1 = 0, hotspot1 = x11;

            if (ninlet2 > 1)
            {
                closest2 = ((xpos-x21) * (ninlet2-1) + width2/2)/width2;
                hotspot2 = x21 +
                    (width2 - IOWIDTH) * closest2 / (ninlet2-1);
            }
            else closest2 = 0, hotspot2 = x21;

            if (closest1 >= noutlet1)
                closest1 = noutlet1 - 1;
            if (closest2 >= ninlet2)
                closest2 = ninlet2 - 1;

            if (canvas_isconnected (x, ob1, closest1, ob2, closest2))
            {
                canvas_setcursor(x, CURSOR_EDITMODE_NOTHING);
                return;
            }
            if (obj_issignaloutlet(ob1, closest1) &&
                !obj_issignalinlet(ob2, closest2))
            {
                if (doit)
                    error("can't connect signal outlet to control inlet");
                canvas_setcursor(x, CURSOR_EDITMODE_NOTHING);
                return;
            }
            if (doit)
            {
                oc = obj_connect(ob1, closest1, ob2, closest2);
                lx1 = x11 + (noutlet1 > 1 ?
                        ((x12-x11-IOWIDTH) * closest1)/(noutlet1-1) : 0)
                             + IOMIDDLE;
                ly1 = y12;
                lx2 = x21 + (ninlet2 > 1 ?
                        ((x22-x21-IOWIDTH) * closest2)/(ninlet2-1) : 0)
                            + IOMIDDLE;
                ly2 = y21;
-                sys_vgui(".x%lx.c create line %d %d %d %d -width %d -tags l%lx\n",
-                    glist_getcanvas(x),
-                        lx1, ly1, lx2, ly2,
-                            (obj_issignaloutlet(ob1, closest1) ? 2 : 1), oc);
+                sys_vgui(".x%lx.c create line %d %d %d %d -width %d -tags l%lx -fill #%06lX\n",
+                    glist_getcanvas(x),
+                        lx1, ly1, lx2, ly2,
+                            (obj_issignaloutlet(ob1, closest1) ? 2 : 1), oc,
+								(((long)oc<<2) & 0x0FFFFFF));
                canvas_setundo(x, canvas_undo_connect,
                    canvas_undo_set_connect(x,
                        canvas_getindex(x, &ob1->ob_g), closest1,
                        canvas_getindex(x, &ob2->ob_g), closest2),
                        "connect");
            }
            else canvas_setcursor(x, CURSOR_EDITMODE_CONNECT);
            return;
        }
    }
    canvas_setcursor(x, CURSOR_EDITMODE_NOTHING);
}

/* g_editor.c 2176: */
/* canvas_connect is called by canvas_undo_disconnect in g_editor.c and is also */
/* the method for 'connect' messages sent to a canvas */
/* canvas_connect creates lines from 0,0 to 0,0 and then calls */
/* canvas_fixlinesfor to set their true endpoints */

void canvas_connect(t_canvas *x, t_floatarg fwhoout, t_floatarg foutno,
    t_floatarg fwhoin, t_floatarg finno)
{
    int whoout = fwhoout, outno = foutno, whoin = fwhoin, inno = finno;
    t_gobj *src = 0, *sink = 0;
    t_object *objsrc, *objsink;
    t_outconnect *oc;
    int nin = whoin, nout = whoout;
    if (paste_canvas == x) whoout += paste_onset, whoin += paste_onset;
    for (src = x->gl_list; whoout; src = src->g_next, whoout--)
        if (!src->g_next) goto bad; /* bug fix thanks to Hannes */
    for (sink = x->gl_list; whoin; sink = sink->g_next, whoin--)
        if (!sink->g_next) goto bad;

        /* check they're both patchable objects */
    if (!(objsrc = pd_checkobject(&src->g_pd)) ||
        !(objsink = pd_checkobject(&sink->g_pd)))
            goto bad;

        /* if object creation failed, make dummy inlets or outlets
        as needed */
    if (pd_class(&src->g_pd) == text_class && objsrc->te_type == T_OBJECT)
        while (outno >= obj_noutlets(objsrc))
            outlet_new(objsrc, &s_);
    if (pd_class(&sink->g_pd) == text_class && objsink->te_type == T_OBJECT)
        while (inno >= obj_ninlets(objsink))
            inlet_new(objsink, &objsink->ob_pd, &s_, &s_);

    if (!(oc = obj_connect(objsrc, outno, objsink, inno))) goto bad;
    if (glist_isvisible(x))
    {
        sys_vgui(".x%lx.c create line %d %d %d %d -width %d -tags l%lx\n",
            glist_getcanvas(x), 0, 0, 0, 0,
            (obj_issignaloutlet(objsrc, outno) ? 2 : 1),oc);
        canvas_fixlinesfor(x, objsrc);
    }
    return;

bad:
    post("%s %d %d %d %d (%s->%s) connection failed",
        x->gl_name->s_name, nout, outno, nin, inno,
            (src? class_getname(pd_class(&src->g_pd)) : "???"),
            (sink? class_getname(pd_class(&sink->g_pd)) : "???"));
}


/* g_graph.c 634: */
/* Called by canvas_setgraph, canvas_dofont in g_editor.c; graph_bounds, graph_xticks, graph_yticks, */
/* graph_xlabel, graph_ylabel, and graph_displace in g_graph.c */

void glist_redraw(t_glist *x)
{
    if (glist_isvisible(x))
    {
            /* LATER fix the graph_vis() code to handle both cases */
        if (glist_istoplevel(x))
        {
            t_gobj *g;
            t_linetraverser t;
            t_outconnect *oc;
            for (g = x->gl_list; g; g = g->g_next)
            {
                gobj_vis(g, x, 0);
                gobj_vis(g, x, 1);
            }
                /* redraw all the lines */
            linetraverser_start(&t, x);
            while (oc = linetraverser_next(&t))
                sys_vgui(".x%lx.c coords l%lx %d %d %d %d\n",
                    glist_getcanvas(x), oc,
                        t.tr_lx1, t.tr_ly1, t.tr_lx2, t.tr_ly2);
+				sys_vgui(".x%lx.c itemconfigure l%lx -fill #%06lX\n",
+					glist_getcanvas(x), oc, (((long)oc<<2) ^ 0x0FFFFFF));
            canvas_drawredrect(x, 0);
            if (x->gl_goprect)
            {
                post("draw it");
                canvas_drawredrect(x, 1);
            }
        }
        if (x->gl_owner && glist_isvisible(x->gl_owner))
        {
            graph_vis(&x->gl_gobj, x->gl_owner, 0);
            graph_vis(&x->gl_gobj, x->gl_owner, 1);
        }
    }
}

/* fin line_drawing_in_pd.c */
