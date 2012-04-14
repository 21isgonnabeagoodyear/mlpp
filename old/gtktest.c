/*
 * An example of using GtkGLExt in C
 *
 * Written by Davyd Madeley <davyd@madeley.id.au> and made available under a
 * BSD license.
 *
 * This is purely an example, it may eat your cat and you can keep both pieces.
 *
 * Compile with:
 *    gcc -o gtkglext-example `pkg-config --cflags --libs gtk+-2.0 gtkglext-1.0 gtkglext-x11-1.0` gtkglext-example.c
 */

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>

#include <math.h>
#include <stdlib.h>

float boxv[][3] = {
	{ -0.5, -0.5, -0.5 },
	{  0.5, -0.5, -0.5 },
	{  0.5,  0.5, -0.5 },
	{ -0.5,  0.5, -0.5 },
	{ -0.5, -0.5,  0.5 },
	{  0.5, -0.5,  0.5 },
	{  0.5,  0.5,  0.5 },
	{ -0.5,  0.5,  0.5 }
};
#define ALPHA 0.5

static float ang = 30.;

float cursorx, cursory, pressure;
float precursorx, precursory, prepressure;

GLuint texid = -1;
static gboolean
expose (GtkWidget *da, GdkEventExpose *event, gpointer user_data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (da);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (da);

	// g_print (" :: expose\n");

	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
	{
		g_assert_not_reached ();
	}

	/* draw in here */
/*	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	
	glRotatef (ang, 1, 0, 1);
	// glRotatef (ang, 0, 1, 0);
	// glRotatef (ang, 0, 0, 1);

	glShadeModel(GL_FLAT);

#if 0
	glBegin (GL_QUADS);
	glColor4f(0.0, 0.0, 1.0, ALPHA);
	glVertex3fv(boxv[0]);
	glVertex3fv(boxv[1]);
	glVertex3fv(boxv[2]);
	glVertex3fv(boxv[3]);

	glColor4f(1.0, 1.0, 0.0, ALPHA);
	glVertex3fv(boxv[0]);
	glVertex3fv(boxv[4]);
	glVertex3fv(boxv[5]);
	glVertex3fv(boxv[1]);
	
	glColor4f(0.0, 1.0, 1.0, ALPHA);
	glVertex3fv(boxv[2]);
	glVertex3fv(boxv[6]);
	glVertex3fv(boxv[7]);
	glVertex3fv(boxv[3]);
	
	glColor4f(1.0, 0.0, 0.0, ALPHA);
	glVertex3fv(boxv[4]);
	glVertex3fv(boxv[5]);
	glVertex3fv(boxv[6]);
	glVertex3fv(boxv[7]);
	
	glColor4f(1.0, 0.0, 1.0, ALPHA);
	glVertex3fv(boxv[0]);
	glVertex3fv(boxv[3]);
	glVertex3fv(boxv[7]);
	glVertex3fv(boxv[4]);
	
	glColor4f(0.0, 1.0, 0.0, ALPHA);
	glVertex3fv(boxv[1]);
	glVertex3fv(boxv[5]);
	glVertex3fv(boxv[6]);
	glVertex3fv(boxv[2]);

	glEnd ();
#endif

	glBegin (GL_LINES);
	glColor3f (1., 0., 0.);
	glVertex3f (0., 0., 0.);
	glVertex3f (1., 0., 0.);
	glEnd ();
	
	glBegin (GL_LINES);
	glColor3f (0., 1., 0.);
	glVertex3f (0., 0., 0.);
	glVertex3f (0., 1., 0.);
	glEnd ();
	
	glBegin (GL_LINES);
	glColor3f (0., 0., 1.);
	glVertex3f (0., 0., 0.);
	glVertex3f (0., 0., 1.);
	glEnd ();

	glBegin(GL_LINES);
	glColor3f (1., 1., 1.);
	glVertex3fv(boxv[0]);
	glVertex3fv(boxv[1]);
	
	glVertex3fv(boxv[1]);
	glVertex3fv(boxv[2]);
	
	glVertex3fv(boxv[2]);
	glVertex3fv(boxv[3]);
	
	glVertex3fv(boxv[3]);
	glVertex3fv(boxv[0]);
	
	glVertex3fv(boxv[4]);
	glVertex3fv(boxv[5]);
	
	glVertex3fv(boxv[5]);
	glVertex3fv(boxv[6]);
	
	glVertex3fv(boxv[6]);
	glVertex3fv(boxv[7]);
	
	glVertex3fv(boxv[7]);
	glVertex3fv(boxv[4]);
	
	glVertex3fv(boxv[0]);
	glVertex3fv(boxv[4]);
	
	glVertex3fv(boxv[1]);
	glVertex3fv(boxv[5]);
	
	glVertex3fv(boxv[2]);
	glVertex3fv(boxv[6]);
	
	glVertex3fv(boxv[3]);
	glVertex3fv(boxv[7]);
	glEnd();

	glPopMatrix ();



*/
/*	glBegin(GL_TRIANGLE_STRIP);
	glColor4f(0, 0,0, 0.01);
	glVertex3f(-1,-1, 0);
	glVertex3f(-1,1,0);
	glVertex3f(1,-1,0);
	glVertex3f(1,1,0);
	glEnd();*/



	int i;
	int numsamples = sqrt((cursorx - precursorx)*(cursorx - precursorx) + (cursory - precursory)* (cursory - precursory) )*0.5 +1;
	glBindTexture(GL_TEXTURE_2D, texid);
//printf("texid%d\n", texid);
	for(i=0;i<numsamples;i++)
	{
		float cx = (cursorx * i + precursorx * (numsamples-i) )/ numsamples;
		float cy = (cursory * i + precursory * (numsamples-i) )/ numsamples;
		float p = 0.01+(pressure * i + prepressure * (numsamples-i) )/ numsamples;
		/*
		glBegin(GL_TRIANGLE_STRIP);
		glColor4f(1, 1,1, p);
		glVertex3f(2*cx/gdk_window_get_width(da->window)-0.01-1,          1-2*cy/gdk_window_get_height(da->window)-0.01, 0);
		glVertex3f(2*cx/gdk_window_get_width(da->window)+0.01-1,          1-2*cy/gdk_window_get_height(da->window)-0.01, 0);
		glVertex3f(2*cx/gdk_window_get_width(da->window)-0.01-1,          1-2*cy/gdk_window_get_height(da->window)+0.01, 0);
		glVertex3f(2*cx/gdk_window_get_width(da->window)+0.01-1,          1-2*cy/gdk_window_get_height(da->window)+0.01, 0);
		glEnd();*/
		glBegin(GL_TRIANGLE_FAN);
		glTexCoord2f(0.5,0.5);
		glColor4f(1, 1,1, p);
		glVertex3f(2*cx/gdk_window_get_width(da->window)-1     , 1-2*cy/gdk_window_get_height(da->window), 0);
		glColor4f(1, 1,1, 0);
		int j;
		for(j=0;j<11;j++)
		{
			glTexCoord2f(sin(j*3.14/5)*0.5+0.5     , cos(j*3.14/5)*0.5+0.5);
			glVertex3f(2*cx/gdk_window_get_width(da->window)-1  +0.1*sin(j*3.14/5)     , 1-2*cy/gdk_window_get_height(da->window) + 0.1*cos(j*3.14/5), 0);
			//glVertex3f(2*cx/gdk_window_get_width(da->window)-1  +p*0.1*sin(j*3.14/5)     , 1-2*cy/gdk_window_get_height(da->window) + p*0.1*cos(j*3.14/5), 0);
		}

		glEnd();
	}
	glBegin(GL_QUADS);
	glColor4f(1,1,0.1,1);
	glTexCoord2f(0,0);glVertex3f(0,0,0);
	glTexCoord2f(0,1);glVertex3f(0,1,0);
	glTexCoord2f(1,1);glVertex3f(1,1,0);
	glTexCoord2f(1,0);glVertex3f(1,0,0);
	glEnd();



	precursorx = cursorx;
	precursory = cursory;
	prepressure = pressure;




	if (gdk_gl_drawable_is_double_buffered (gldrawable))
		gdk_gl_drawable_swap_buffers (gldrawable);

	else
		glFlush ();

	gdk_gl_drawable_gl_end (gldrawable);

	return TRUE;
}

static gboolean
configure (GtkWidget *da, GdkEventConfigure *event, gpointer user_data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (da);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (da);

	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
	{
		g_assert_not_reached ();
	}

	glLoadIdentity();
	glViewport (0, 0, da->allocation.width, da->allocation.height);
	glOrtho (-10,10,-10,10,-20050,10000);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glScalef (10., 10., 10.);

//////////////////////
	if(texid == -1)
	{
	glShadeModel(GL_SMOOTH);
	glEnable (GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_3D);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glGenTextures(1, &texid);
	printf("got texture %d\n", texid);
	//float texdata[1024][1024][4];
	float *texdata = malloc(sizeof(float)*512*512*4);
	int i,j;
	for(i=0;i<512;i++)
	{
		for(j=0;j<512;j++)
		{
			texdata[(i*512+j)*4] = (float)((i&j)==0)*255;
			texdata[(i*512+j)*4+1] = 0.1;
			texdata[(i*512+j)*4+3] = 1;
		}
	}
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 512,512,0,GL_RGBA, GL_FLOAT,texdata);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT); // Linear Filtering
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT); // Linear Filtering
//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, texid);
	free(texdata);
	}
/////////////////////
	gdk_gl_drawable_gl_end (gldrawable);

	return TRUE;
}

static gboolean
rotate (gpointer user_data)
{
	GtkWidget *da = GTK_WIDGET (user_data);

	ang++;

	gdk_window_invalidate_rect (da->window, &da->allocation, FALSE);
	gdk_window_process_updates (da->window, FALSE);

	return TRUE;
}

int mousemove(GtkWidget *widget, GdkEventMotion *event)
{
	//printf("clicked\n");
	double stuff[1000];
	gdk_device_get_state( event->device,event->window, stuff, NULL);
	int i;
	//for(i=0;i<3;i++)
	//	printf("%f ",stuff[i]);
	cursorx = stuff[0];
	cursory = stuff[1];
	pressure = stuff[2];
	//gdk_input_window_get_pointer(event->window, event->device, NULL, NULL, &pressure, NULL,NULL,NULL);
	gdk_window_invalidate_rect (event->window, &widget->allocation, FALSE);
	return 1;

}

int
main (int argc, char **argv)
{
	GtkWidget *window;
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
			G_CALLBACK (configure), NULL);
	g_signal_connect (da, "expose-event",
			G_CALLBACK (expose), NULL);

	gtk_widget_show_all (window);

	g_timeout_add (1000 / 60, rotate, da);

	gtk_main ();
}
