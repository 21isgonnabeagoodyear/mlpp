
//gcc -g -o main `pkg-config --cflags --libs gtk+-2.0 gtkglext-1.0 gtkglext-x11-1.0` external.c

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>

#include <math.h>
#include <stdlib.h>

#include "compositor.c"
static float cursorx, cursory, pressure;

static gboolean cbredraw(GtkWidget *da, GdkEventExpose *event, gpointer user_data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (da);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (da);

	// g_print (" :: expose\n");

	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
	{
		g_assert_not_reached ();
	}
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//do shit here
	c_draw();

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
	double stuff[1000];
	gdk_device_get_state( event->device,event->window, stuff, NULL);
	int i;
	//if(stuff[2] > 0)
		//printf("pressure:%f \n",stuff[2]);
	cursorx = stuff[0];
	cursory = stuff[1];
	pressure = stuff[2];
	//gdk_input_window_get_pointer(event->window, event->device, NULL, NULL, &pressure, NULL,NULL,NULL);
	gdk_window_invalidate_rect (event->window, &widget->allocation, FALSE);
	return 1;

}
int main( int  argc, char **argv)
{
	GtkWidget *window;
	gtk_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);
//(from example)
	GtkWidget *da;
	GdkGLConfig *glconfig;

	gtk_init (&argc, &argv);
	gtk_gl_init (&argc, &argv);


	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
	da = gtk_drawing_area_new ();

	gtk_container_add (GTK_CONTAINER (window), da);
	g_signal_connect_swapped (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);
	gtk_widget_set_events (da, GDK_EXPOSURE_MASK);

	gtk_widget_show (window);

	/* prepare GL */
	glconfig = gdk_gl_config_new_by_mode (
			GDK_GL_MODE_RGB |
			GDK_GL_MODE_DEPTH |
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
//(/from example)



	//initialize input devices
	GList *devices = gdk_devices_list();
	while(devices->next != NULL)
	{
		printf("found input device\n");
		if(!gdk_device_set_mode(devices->data, GDK_MODE_SCREEN))
			printf("failed to enable\n");
		devices = devices->next;
	}
	g_signal_connect(window, "motion-notify-event", G_CALLBACK(mousemove), NULL);

	gdk_input_set_extension_events(window->window, 0xffffffff, GDK_EXTENSION_EVENTS_ALL);

	c_init();
	gtk_main();
}
