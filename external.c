
//gcc -g -o mlpp `pkg-config --cflags --libs gtk+-2.0 gtkglext-1.0 gtkglext-x11-1.0` external.c

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <gdk/gdkkeysyms.h>

#include <math.h>
#include <stdlib.h>

#include "compositor.c"
static float cursorx, cursory, pressure;
static float dragpx, dragpy;
static float precursorx, precursory, prepressure;
static float zoom;

static gboolean cbredraw(GtkWidget *da, GdkEventExpose *event, gpointer user_data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (da);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (da);

	// g_print (" :: expose\n");

	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
	{
		g_assert_not_reached ();
	}
//	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//glVertex3f(2*x/gdk_window_get_width(da->window)-1  +0.1*sin(j*3.14/5)     , 1-2*y/gdk_window_get_height(da->window) + 0.1*cos(j*3.14/5), 0);
	//do shit here
	c_windowsize(gdk_window_get_width(da->window), gdk_window_get_height(da->window));
	
	c_draw();
	c_viscursor(cursorx/gdk_window_get_width(da->window), cursory/gdk_window_get_height(da->window));
//	c_viswheel();
	gdk_gl_drawable_gl_end (gldrawable);
	if (gdk_gl_drawable_is_double_buffered (gldrawable))
		gdk_gl_drawable_swap_buffers (gldrawable);

	else
		glFlush ();


	return TRUE;
}


static gboolean cbconfigure(GtkWidget *da, GdkEventConfigure *event, gpointer user_data)
{
	//(from example)
	GdkGLContext *glcontext = gtk_widget_get_gl_context (da);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (da);

	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
	{
		g_assert_not_reached ();
	}

	glLoadIdentity();
	glViewport (0, 0, da->allocation.width, da->allocation.height);
	//glOrtho (-10,10,-10,10,-20050,10000);
	glOrtho (-1,1,-1,1,-20050,10000);
	gdk_gl_drawable_gl_end (gldrawable);
	//(/from example)
}
static gboolean  mousemove(GtkWidget *widget, GdkEventMotion *event)
{
	//printf("clicked\n");
//	if(event->device->num_axes < 2)
//		return 1;
	double stuff[1000];
	gdk_device_get_state( event->device,event->window, stuff, NULL);
	int i;
	//if(stuff[2] > 0)
	cursorx = stuff[0];
	cursory = stuff[1];
	pressure = stuff[2];
	//gdk_input_window_get_pointer(event->window, event->device, NULL, NULL, &pressure, NULL,NULL,NULL);

if(event->device->source == GDK_SOURCE_MOUSE&& event->state & GDK_BUTTON1_MASK)
	pressure = 1;
//printf("mvd\n");
	if(event->state & GDK_BUTTON2_MASK)
	{
		float dragtx = cursorx/gdk_window_get_width(widget->window) - dragpx;
		float dragty = cursory/gdk_window_get_height(widget->window) - dragpy;
		c_translate(dragtx, dragty);
		//c_rotation(atan(-(cursorx - gdk_window_get_width(widget->window)/2)/( cursory-gdk_window_get_width(widget->window)/2)));	
		//c_rotation(atan((dragtx-dragpx)/(dragty-dragpy)));	
//		c_rotation(atan((dragty-dragpy)/(dragtx-dragpx)));	
		
	}
	dragpx = cursorx/gdk_window_get_width(widget->window);
	dragpy = cursory/gdk_window_get_height(widget->window);






	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

	// g_print (" :: expose\n");

	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
	{
		g_assert_not_reached ();
	}
	//do shit here
	c_windowsize(gdk_window_get_width(widget->window), gdk_window_get_height(widget->window));
	if(pressure > 0)
	{
		int i;
		int numsamples = ceil(sqrt((cursorx - precursorx)*(cursorx - precursorx) + (cursory - precursory)* (cursory - precursory) )*0.5);
		//printf("texid%d\n", texid);
		for(i=0;i<numsamples;i++)
		{
			float cx = (cursorx * i + precursorx * (numsamples-i) )/ numsamples;
			float cy = (cursory * i + precursory * (numsamples-i) )/ numsamples;
			float p = 0.01+(pressure * i + prepressure * (numsamples-i) )/ numsamples;
			c_paint(cx/gdk_window_get_width(widget->window),cy/gdk_window_get_height(widget->window),p*p);
		}

	}

	precursorx = cursorx;
	precursory = cursory;
	prepressure = pressure;

	gdk_gl_drawable_gl_end (gldrawable);


















	gdk_window_invalidate_rect (event->window, NULL, TRUE);
	return 1;

}
static gboolean cbscroll(GtkWidget *widget, GdkEventScroll *event, gpointer notused)
{
	if(event->direction == GDK_SCROLL_UP)
		zoom /= 0.9;
	else if(event->direction == GDK_SCROLL_DOWN)
		zoom *= 0.9;
	zoom = zoom>1000?1000:zoom;
	zoom = zoom<0.1?0.1:zoom;
	c_zoom(zoom);
	gdk_window_invalidate_rect (event->window, NULL, TRUE);
	return FALSE;

}
static gboolean cbclicked(GtkWidget *widget, GdkEventButton *event, gpointer notused)
{
//printf("clicked %d\n", event->button);
	float rx, ry;
	//static float dragtx, dragty;
	if(event->axes ==NULL)
	{
		rx = event->x;
		ry = event->y;
//		printf("x:%f\n", rx);
	}
	else
	{//printf("derp\n");
		rx = cursorx;//event->axes[0];
		ry = cursory;//event->axes[1];
//		printf("x:%f\n", rx);
//		printf("y:%f\n", ry);
//		printf("x:%d\n", gdk_window_get_width(widget->window));
	}


	if(event->button == 3)
	{
		if(event->type == GDK_BUTTON_PRESS)
			c_showwheel(rx/gdk_window_get_width(widget->window),ry/gdk_window_get_height(widget->window));
		else //if(event->type == GDK_BUTTON_RELEASE)
		{
			c_hidewheel();
			c_droppersample(rx/gdk_window_get_width(widget->window), ry/gdk_window_get_height(widget->window));
		}
	}
	else if(event->button == 5)
		zoom /= 0.9;
	else if(event->button == 4)
		zoom *= 0.9;

	zoom = zoom>1000?1000:zoom;
	zoom = zoom<0.10?0.10:zoom;
	c_zoom(zoom);


	gdk_window_invalidate_rect (event->window, NULL, TRUE);
	return FALSE;
}
gboolean cbkeypress(GtkWidget *widget, GdkEventKey *event, gpointer notused)
{
//	printf("key %d\n", event->keyval);
	//if(event->keyval == GDK_bracketleft)
	static float alpha = 0.5;
	if(event->keyval == 65361)
		c_rotate(PI/32);
	else if(event->keyval == 65363)
		c_rotate(-PI/32);
	else if(event->keyval >=49 && event->keyval <=57 )//keys 1,2,3,4
		b_switch(event->keyval - 49);
	else if(event->keyval >=65470 && event->keyval <=65474 )//f1-f4
		c_selectlayer(event->keyval - 65470);
	else if(event->keyval ==65535)//del
		b_mode(1);
	else if(event->keyval ==65293)//enter
		b_mode(0);
	else if(event->keyval == 65362)//up
		b_alpha(alpha =(alpha*1.5)>5?5:(alpha*1.5));
	else if(event->keyval == 65364)//down
		b_alpha(alpha =(alpha/1.5)<0.0001?0.0001:(alpha/1.5));
	else if(event->keyval ==44)//enter
		b_scale(0.75);
	else if(event->keyval ==46)//enter
		b_scale(1.5);
	else if(event->keyval ==93)//enter
		c_layeropacity(0.05);
	else if(event->keyval ==91)//enter
		c_layeropacity(-0.05);
	gdk_window_invalidate_rect (event->window, NULL, TRUE);
	return FALSE;

}
int main( int  argc, char **argv)
{
	GtkWidget *window;
	gtk_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);
	//(from example)
	GtkWidget *da;
	GdkGLConfig *glconfig;

/*
	GtkBuilder *build = gtk_builder_new();
	gtk_builder_add_from_file(build, "window.builder", NULL);
	da = GTK_WIDGET(gtk_builder_get_object(build, "da"));
	window = GTK_WIDGET(gtk_builder_get_object(build, "win"));
	gtk_builder_connect_signals(build, NULL);
*/


	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
	da = gtk_drawing_area_new ();

	GtkWidget *pane = gtk_hpaned_new();
	gtk_container_add (GTK_CONTAINER (window), pane);

	GtkWidget *button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(pane), button);


	gtk_container_add (GTK_CONTAINER (pane), da);
	g_signal_connect_swapped (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);
	gtk_widget_set_events (da, GDK_EXPOSURE_MASK);

	gtk_widget_show (window);

	/* prepare GL */
	glconfig = gdk_gl_config_new_by_mode (
			GDK_GL_MODE_RGB |
			//		GDK_GL_MODE_DEPTH |
			GDK_GL_MODE_DOUBLE);

	if (!glconfig)
	{
		g_assert_not_reached ();
	}

	if (!gtk_widget_set_gl_capability (da, glconfig, NULL, TRUE,
				GDK_GL_RGBA_TYPE))
	{
		g_assert_not_reached ();
	}

	g_signal_connect (da, "configure-event",
			G_CALLBACK (cbconfigure), NULL);
	g_signal_connect (da, "expose-event",
			G_CALLBACK (cbredraw), NULL);

	gtk_widget_show_all (window);
	gtk_window_fullscreen(GTK_WINDOW(window));
	//(/from example)



	//initialize input devices
	GList *devices = gdk_devices_list();
	while(devices->next != NULL)
	{
		int numaxes = gdk_device_get_n_axes(devices->data);
		printf("found input device %s\n", gdk_device_get_name(devices->data));
//		if(numaxes >= 2)
//		{
			if(!gdk_device_set_mode(devices->data, GDK_MODE_SCREEN))
				printf("failed to enable\n");
			else
				printf("...enabled\n");
//		}
//		else
//			printf("...it's probably not a tablet\n");
		devices = devices->next;
	}
	gtk_widget_add_events(GTK_WIDGET(da), GDK_SCROLL_MASK|GDK_BUTTON_PRESS_MASK|GDK_KEY_PRESS_MASK);
	g_signal_connect(da, "motion-notify-event", G_CALLBACK(mousemove), NULL);
	g_signal_connect (da, "scroll-event",G_CALLBACK (cbscroll), NULL);//FIXME:doesn't work with extension events enabled
	g_signal_connect (da, "button-press-event",G_CALLBACK (cbclicked), NULL);//FIXME:doesn't work with extension events enabled
	g_signal_connect (da, "button-release-event",G_CALLBACK (cbclicked), NULL);//FIXME:doesn't work with extension events enabled
	g_signal_connect (window, "key-press-event",G_CALLBACK (cbkeypress), NULL);//FIXME:doesn't work with extension events enabled
	gdk_input_set_extension_events(da->window, 0xffffffff, GDK_EXTENSION_EVENTS_ALL);

	c_init();
	gtk_main();
}
