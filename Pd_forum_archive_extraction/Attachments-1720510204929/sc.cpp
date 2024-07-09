#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <vector>

/*
 * how to build:
 *
 * $ g++ sc.cpp `fltk-config --cxxflags` `fltk-config --ldflags`
 *
 */

using std::vector;

class Obj {
private:
	int dx; int dy;
public:
	int x; int y;
	int w; int h;
	int inlets; int outlets;
	Obj() { x=y=w=h=inlets=0; }
	Obj(int w_, int h_) { x=y=0; w=w_; h=h_; }
	Obj(int x_, int y_, int w_, int h_) { x=x_; y=y_; w=w_; h=h_; }
	Obj(int x_, int y_, int w_, int h_, int in, int out)
	{ x=x_; y=y_; w=w_; h=h_; inlets=in; outlets=out; }
	int getInletX(int n) { return x+((inlets>1)?(w/(inlets-1)*n):0); }
	int getInletY(int n) { return y; }
	int getOutletX(int n) { return x+((outlets>1)?(w/(outlets-1)*n):0); }
	int getOutletY(int n) { return y+h; }
	void move(int x_, int y_) { x=x_-dx; y=y_-dy; }
	void r() { dx=dy=0; }
	int hit(int x_, int y_)	{
		int k = (x_>=x&&x_<(x+w)&&y_>=y&&y_<=(y+h));
		if(k) { dx=x_-x; dy=y_-y; }
		return k;
	}
};

class Wire {
public:
	int px[6]; int py[6];
	Obj* o1; Obj* o2;
	int k1; int k2;
	
	Wire(Obj* o_1, int xk1, Obj* o_2, int xk2) {
		o1=o_1; o2=o_2; k1=xk1; k2=xk2;
		zap();
	}
	
	void zap() {
		const int min_b = 10;
		int seg3h = 0;
		px[0]=o1->getOutletX(k1); py[0]=o1->getOutletY(k1);
		px[1]=px[0]; py[1]=py[0]+min_b;
		
		px[5]=o2->getInletX(k2); py[5]=o2->getInletY(k2);
		px[4]=px[5]; py[4]=py[5]-min_b;

		int ydelta = py[4]-py[1];
		int xdelta = px[4]-px[1];
		
		px[2]=px[3] = px[0]+(xdelta/2);
		if(ydelta >= 0) {
			py[1] += (ydelta/2);
			py[4] -= (ydelta/2);
		}
		py[2] = py[1];
		py[3] = py[4];
	}

	int hit(Obj* o) { return (o==o1||o==o2); }
};

class Fl_Test : public Fl_Widget {
public:
	int dn;
	int hit; int hitw;
	vector<Obj*> vec;
	vector<Wire*> wire;
	
	Fl_Test(int x, int y, int w, int h, const char *label)
	: Fl_Widget(x, y, w, h, label) {
		dn = 0;
		hit = -1;
		hitw = -1;
	}

	void X(int x, int y) {
		fl_line_style(FL_SOLID);
		fl_color(FL_BLACK);
		const int sz=2;
		fl_line(x-sz,y-sz,x+sz,y+sz);
		fl_line(x+sz,y-sz,x-sz,y+sz);
	}

	void draw_obj(Obj* o) {
		int i,tx,ty;
		fl_line_style(FL_DOT);
		fl_color(FL_RED);
		fl_rect(o->x, o->y, o->w, o->h);
		fl_line_style(FL_SOLID);
		fl_color(FL_BLUE);
		for(i=0; i<o->inlets; i++) {
			tx = o->getInletX(i); ty = o->getInletY(i);
			fl_rect(tx-1, ty-1, 3, 3);
		}
		for(i=0; i<o->outlets; i++) {
			tx = o->getOutletX(i); ty = o->getOutletY(i);
			fl_rect(tx-1, ty-1, 3, 3);
		}

	}

	void draw_wire(Wire* w) {
		fl_line_style(FL_DOT);
		fl_color(FL_GREEN);
		X(w->px[0], w->py[0]);
		for(int i=1; i<6; i++) {
			fl_line(w->px[i-1], w->py[i-1], w->px[i], w->py[i]);
			X(w->px[i], w->py[i]);
		}
	}
	
	void draw() {
		int i;
		fl_rectf(0, 0, w(), h(), 0xff, 0xff, 0xff);
		for(i=0; i<vec.size(); i++) draw_obj(vec[i]);
		for(i=0; i<wire.size(); i++) draw_wire(wire[i]);
	}

	int handle(int event) {
		switch(event) {
		case FL_PUSH: {
			for(int i=0; i<vec.size(); i++) {
				if(vec[i]->hit(Fl::event_x(), Fl::event_y())) {
					hit = i;
					for(int z=0; z<wire.size(); z++)
						if(wire[z]->hit(vec[i]))
							hitw = z;
				}
			}
			dn = 1;
			redraw();
			} return 1;
		case FL_DRAG: {
			if (dn && hit >= 0) {
				vec[hit]->move(Fl::event_x(), Fl::event_y());
				/*if(hitw >= 0)*/
				for(int z=0; z<wire.size(); z++) wire[z]->zap();
				redraw();
			}
			} return 1;
		case FL_RELEASE:
			if (dn) {
				dn = 0;
				hit = hitw = -1;
				redraw();
				do_callback(); //stop!
			}
			return 1;
		default:
			return Fl_Widget::handle(event);
		}
	}
};

int main(int argc, char **argv) {
	Fl_Window *window = new Fl_Window(500,380);
	Fl_Test *box = new Fl_Test(0,0,window->w(),window->h(),"box.1");
	box->vec.push_back(new Obj(10,10,80,20,1,2));
	box->vec.push_back(new Obj(70,90,80,20,2,2));
	box->vec.push_back(new Obj(180,130,80,20,1,5));
	box->wire.push_back(new Wire(box->vec[0],0, box->vec[1],0));
	box->wire.push_back(new Wire(box->vec[2],4, box->vec[1],0));
	window->end();
	window->show(argc, argv);
	return Fl::run();
}
