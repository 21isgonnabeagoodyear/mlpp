//g++ `fltk-config --use-gl --use-images --optim --cxxflags --ldflags --libs` -lGL fltktest.cpp -o t


//fltk-config --use-forms --use-gl --use-images --compile filename.cpp

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/gl.h>
#include <iostream>

class testgl:public Fl_Gl_Window
{
public:
	testgl():Fl_Gl_Window::Fl_Gl_Window(330,190)
	{
		size_range(100,100);
	}
	void draw()
	{
		static int col = 0;
		col ++;
		glBegin(GL_TRIANGLES);
		glColor3f((col+66)%100*1.0/100,(col+33)%100*1.0/100,col%100*1.0/100);
		glVertex3f(0,1,0);
		glVertex3f(1,1,0);
		glVertex3f(0,0,0);
		glEnd();
	}
	int handle(int event)
	{
		std::cout << event<< "\n";//fltk::event_pressure()<<"\n";
		return 0;
	}


};
/*
class testglnotgl:public Fl_Window
{
public:
	testglnotgl():Fl_Window::Fl_Window(330,190)
	{
		;
	}
	void draw()
	{
		gl_start();
		glBegin(GL_TRIANGLES);
		glColor3f(1,1,0);
		glVertex3f(0,1,0);
		glVertex3f(1,1,1);
		glVertex3f(0,0,0);
		glEnd();
		gl_finish();
	}


};*/


int main(int argc, char *argv[]) {
	Fl::gl_visual(FL_RGB);//enable opengl
	//Fl_Window* w = new Fl_Window(330, 190);
	//Fl_Window *w = new testglnotgl;
	Fl_Window *w = new testgl;
	new Fl_Button(110, 130, 100, 35, "Okay");
	w->end();
	w->show(argc, argv);
	return Fl::run();
}
