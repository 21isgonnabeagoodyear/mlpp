
//#define LAYERSIZE 2048
//#define LAYERSIZE 4096
//#define LAYERSIZE 1024
#define LAYERSIZE 2048
#define NUMLAYERS 4
#define NUMBRUSHES 10
#define PI 3.1415926

typedef struct
{
	GLuint texid;
	float opacity;
} layer_t;

typedef struct
{
	GLuint texid;
	float r,g,b,a;
	float size;
	char softedge;
	char erase;
	char scalepressure;
} brush_t;


static layer_t layers[NUMLAYERS];
static GLuint fbo;
static int currentlayer = 0;
static float windoww, windowh, windowa;

static brush_t brushes[NUMBRUSHES];
static int selectedbrush = 0;

static float zoom = 1;
static float panx = 0, pany = 0;//TODO:implement this
static float rotation = 0;


static GLuint wheeltexture;
static int renderwheel = 0;
static float wheelx, wheely;

static void target(GLuint texid)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);//switch fbos
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texid, 0);//attach texture
	int fbores = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(fbores != GL_FRAMEBUFFER_COMPLETE)
		printf("incomplete framebuffer %d\n", fbores);
	glViewport(0,0,LAYERSIZE,LAYERSIZE);
}
static void untarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);//switch to default fbo
	glViewport(0,0,windoww,windowh);
}

static void drawlayer(int ind)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, layers[ind].texid);
	glBegin(GL_QUADS);
	glColor4f(1,1,1,layers[ind].opacity);
//	printf("op %d %f\n", ind, layers[ind].opacity);
	float xt, yt;
	float rot = rotation;
	xt = -1*zoom;
	yt = -1*zoom;
	glTexCoord2f(0,0);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa +panx,cos(rot)*yt-sin(rot)*xt + pany,0);
	xt = -1*zoom;
	yt = 1*zoom;
	glTexCoord2f(0,1);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa + panx,cos(rot)*yt-sin(rot)*xt + pany,0);
	xt = 1*zoom;
	yt = 1*zoom;
	glTexCoord2f(1,1);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa + panx,cos(rot)*yt-sin(rot)*xt + pany,0);
	xt = 1*zoom;
	yt = -1*zoom;
	glTexCoord2f(1,0);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa + panx,cos(rot)*yt-sin(rot)*xt +pany,0);
	glEnd();
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
}
static void drawwheel()
{
	glBindTexture(GL_TEXTURE_2D, wheeltexture);
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1,1,1,1);
	glTexCoord2f(0,0);
	glVertex3f(-0.8*windowa+wheelx,-0.8+wheely,0);
	glTexCoord2f(1,0);
	glVertex3f(0.8*windowa+wheelx,-0.8+wheely,0);
	glTexCoord2f(1,1);
	glVertex3f(0.8*windowa+wheelx,0.8+wheely,0);
	glTexCoord2f(0,1);
	glVertex3f(-0.8*windowa+wheelx,0.8+wheely,0);
	glEnd();

}
static void wtos(float *x, float *y)
{
	if(x != NULL)
		//*x = ((*x-0.5)*2/windowa)/zoom-panx/windowa/zoom;
		*x = ((*x-0.5)*2/windowa)/zoom-panx/windowa/zoom;
	if(y!= NULL)
		*y = ((*y-0.5)*-2)/zoom-pany/zoom;
}
void c_init()
{
	int i, j;
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	for(i=0;i<NUMLAYERS;i++)
	{
		static float serpi[LAYERSIZE][LAYERSIZE][4];//causes stack overflow if not static
		glGenTextures(1,&layers[i].texid);
		glBindTexture(GL_TEXTURE_2D, layers[i].texid);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, LAYERSIZE, LAYERSIZE, 0, GL_RGBA, GL_FLOAT, serpi);
		layers[i].opacity = 1;
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, LAYERSIZE, LAYERSIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	glGenFramebuffers(1,&fbo);
	//make some more textures and shit here
//TODO:after this to be removed
/*	glBindTexture(GL_TEXTURE_2D, layers[0].texid);
	static float serpi[LAYERSIZE][LAYERSIZE][4];//causes stack overflow if not static
	for(i=0;i<LAYERSIZE;i++)
	{
		for(j=0;j<LAYERSIZE;j++)
		{
			serpi[i][j][0] = (j+i<LAYERSIZE)?((j&i) == 0):(((LAYERSIZE-j)&(LAYERSIZE-i)) == 0);
			serpi[i][j][3] = 1;
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, LAYERSIZE, LAYERSIZE, 0, GL_RGBA, GL_FLOAT, serpi);
	layers[0].opacity = 1;

	for(i=0;i<LAYERSIZE;i++)
	{
		for(j=0;j<LAYERSIZE;j++)
		{
			serpi[i][j][0]=0;
			serpi[i][j][3]=serpi[i][j][1] = fabs(sin(i*0.1)+cos(j*0.1))*0.5;
		}
	}
	glBindTexture(GL_TEXTURE_2D, layers[1].texid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, LAYERSIZE, LAYERSIZE, 0, GL_RGBA, GL_FLOAT, serpi);
	layers[1].opacity = 1;
	
*/
	static float serpi[LAYERSIZE][LAYERSIZE][4];//causes stack overflow if not static
	//for(i=0;i<LAYERSIZE;i++)
	//	for(j=0;j<LAYERSIZE;j++)
	//		serpi[i][j][0] = serpi[i][j][3]=1;
	glBindTexture(GL_TEXTURE_2D, layers[1].texid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, LAYERSIZE, LAYERSIZE, 0, GL_RGBA, GL_FLOAT, serpi);
	layers[1].opacity = 1;
	glBindTexture(GL_TEXTURE_2D, layers[0].texid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, LAYERSIZE, LAYERSIZE, 0, GL_RGBA, GL_FLOAT, serpi);
	layers[0].opacity = 1;
//initialize wheel texture
	static float serpiw[512][512][4];//causes stack overflow if not static
	glGenTextures(1,&wheeltexture);
	glBindTexture(GL_TEXTURE_2D, wheeltexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	static float reds[] = {1,1,0,0,0,1,1,  1};
	static float greens[] = {0,1,1,1,0,0,0,  0};
	static float blues[] = {0,0,0,1,1,1,0,  0};
	for(i=0;i<512;i++)
	{
		for(j=0;j<512;j++)
		{
			serpiw[i][j][0] =serpiw[i][j][1]=serpiw[i][j][2]=serpiw[i][j][3]=0;
			if((i-256)*(i-256)+(j-256)*(j-256) <256*256 && (i-256)*(i-256)+(j-256)*(j-256) > 50*50)
			{
				float angle = atan((float)(i-256)/(j-256));
				if(j < 256) angle = angle+PI;
				angle += PI/2;
				serpiw[i][j][3] = 1;
				int index = (angle/(PI/3));
				float offset = 1-fmod(angle, PI/3)/(PI/3);
				float brightness = 1-sqrt(((i-256)*(i-256)+(j-256)*(j-256)-50*50)/(float)(256*256));
				serpiw[i][j][0] = (reds[index]*offset + reds[index+1]*(1-offset))+brightness;
				serpiw[i][j][1] = (greens[index]*offset + greens[index+1]*(1-offset))+brightness;
				serpiw[i][j][2] = (blues[index]*offset + blues[index+1]*(1-offset))+brightness;
				int k;
				for(k=0;k<3;k++)if(brightness>0.5)serpiw[i][j][k]+=brightness*2-1;else serpiw[i][j][k]*=brightness*2;
			}
			else if(i < 100)
			{
				serpiw[i][j][3] = 1;
				serpiw[i][j][0]=1;
				serpiw[i][j][1]=1;
				serpiw[i][j][2]=1;
			}
			else if(i > 512-100)
			{
				serpiw[i][j][3] = 1;
				serpiw[i][j][0]=0;
				serpiw[i][j][1]=0;
				serpiw[i][j][2]=0;
			}
			else if(j > 256+100 || j < 256-100 )
			{
				serpiw[i][j][3] = 1;
				serpiw[i][j][0]=0.5;
				serpiw[i][j][1]=0.5;
				serpiw[i][j][2]=0.5;
			}
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 512, 512, 0, GL_RGBA, GL_FLOAT, serpiw);

	int freevram;
	glGetIntegerv(0x9049, &freevram);
	printf("%dkb free vram\n", freevram);
	for(i=0;i<NUMBRUSHES;i++)
	{
		brushes[i].size = 1;
		brushes[i].a= 1;
	}
	brushes[1].scalepressure = 1;
	brushes[1].size = 2;
	brushes[1].r = 0.5;
	brushes[2].g = 0.5;
	brushes[3].erase = 1;
}

void c_draw()
{
//	target(layers[1].texid);
//	drawlayer(0);
	untarget();
	int i;
	for(i=0;i<NUMLAYERS;i++)
		drawlayer(i);
	GLuint err = glGetError();
	if(renderwheel)
		drawwheel();
	if(err != 0)
		printf("gl error %d\n", err);
}

void c_selectlayer(int lay)
{
	currentlayer = lay % NUMLAYERS;
printf("switch layer %i\n", lay);
}
void c_windowsize(float x, float y)
{
	windoww = x;
	windowh = y;
	windowa = y/x;
}
void b_color(float r, float g, float b, float a)
{
	brushes[selectedbrush].r = r;
	brushes[selectedbrush].g = g;
	brushes[selectedbrush].b = b;
	brushes[selectedbrush].a = a;
}
void b_switch(int brush)
{
	selectedbrush = brush % NUMBRUSHES;
//	if(brush >=0 && brush < NUMBRUSHES)
//		selectedbrush = brush;
}
void b_mode(int m)
{
	brushes[selectedbrush].erase = m;
}
void b_alpha(float a)
{
	brushes[selectedbrush].a = a;
}
void b_scale(float s)
{
	brushes[selectedbrush].size *= s;
}
void c_zoom(float zto)
{
//	panx = zoom*panx/zto;
//	pany = zoom*pany/zto;
	zoom = zto;
}
void c_showwheel(float x, float y)
{
	renderwheel = 1;
	wheelx = x*2-1;
	wheely = -(y*2-1);
}
void c_hidewheel()
{
	renderwheel = 0;
}
void c_translate(float dx, float dy)
{
	panx += dx*2;
	pany -= dy*2;
}
void c_rotate(float dtheta)
{
	rotation += dtheta;
}

void c_paint(float x, float y, float pressure)
{

	wtos(&x, &y);
	float tx = -y, ty=x;//don't fucking touch this shit
	y = sin(rotation)*ty - cos(rotation)*tx;
	x = sin(rotation)*tx + cos(rotation)*ty;
	target(layers[currentlayer].texid);
	glBindTexture(GL_TEXTURE_2D,layers[1].texid);
	glBindTexture(GL_TEXTURE_2D,0);//disable texturing
//	glDisable(GL_TEXTURE_2D);
	if(brushes[selectedbrush].erase)
		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
	
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.5,0.5);
	glColor4f(brushes[selectedbrush].r, brushes[selectedbrush].g,brushes[selectedbrush].b, pressure*brushes[selectedbrush].a);
	if(brushes[selectedbrush].erase)
		glColor4f(brushes[selectedbrush].r, brushes[selectedbrush].g,brushes[selectedbrush].b, 0);//blending doesn't work so good with erasing
	glVertex3f(x     , y, 0);
	if(brushes[selectedbrush].softedge)
		glColor4f(brushes[selectedbrush].r, brushes[selectedbrush].g,brushes[selectedbrush].b, 0);
	int j;
	for(j=0;j<51;j++)
	{
		glTexCoord2f(sin(j*PI/25)*0.5+0.5     , cos(j*PI/25)*0.5+0.5);
		//glVertex3f(x  +0.01*sin(j*3.14/5)     , y + 0.01*cos(j*3.14/5), 0);
		//glVertex3f(x  +0.05*sin(j*3.14/5)*pressure     , y + 0.05*cos(j*3.14/5)*pressure, 0);
		if(brushes[selectedbrush].scalepressure)
			glVertex3f(x  +0.1*sin(j*PI/25)*pressure/zoom*brushes[selectedbrush].size     , y + 0.1*cos(j*PI/25)*pressure/zoom*brushes[selectedbrush].size, 0);
		else
			glVertex3f(x  +0.1*sin(j*PI/25)/zoom*brushes[selectedbrush].size     , y + 0.1*cos(j*PI/25)/zoom*brushes[selectedbrush].size, 0);
	}

	glEnd();
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glEnable(GL_TEXTURE_2D);
	untarget();
}
void c_viscursor(float x, float y )
{

//	wtos(&x, &y);
	x = (x*2)-1;
	y = (y*-2)+1;
	glBindTexture(GL_TEXTURE_2D,0);//disable texturing
	glEnable(GL_LINE_STIPPLE);
	if(brushes[selectedbrush].erase)
	{
		glColor4f(1, 1,0.5, 1);
		glLineStipple(1, 0xCCCC);
	}
	else
	{
		glColor4f(brushes[selectedbrush].r, brushes[selectedbrush].g,brushes[selectedbrush].b, 1);
		glLineStipple(1, 0xFFFF);
	}
	glBegin(GL_LINE_STRIP);
	int j;
	for(j=0;j<21;j++)
	{
		glTexCoord2f(sin(j*PI/10)*0.5+0.5     , cos(j*PI/10)*0.5+0.5);
		glVertex3f(x  +0.1*sin(j*PI/10)*windowa*brushes[selectedbrush].size, y + 0.1*cos(j*PI/10)*brushes[selectedbrush].size, 0);
	}

	glEnd();
	glEnable(GL_TEXTURE_2D);
}
void c_droppersample(float x, float y)
{
	float retrieved[4*4];
printf("windoww %f windowh %f", windoww, windowh);
	glReadPixels(x*windoww, (1-y)*windowh, 2,2,GL_RGBA, GL_FLOAT, retrieved);
	retrieved[0] = (retrieved[0]+retrieved[4]+retrieved[8]+retrieved[12])/4;
	retrieved[1] = (retrieved[1]+retrieved[5]+retrieved[9]+retrieved[13])/4;
	retrieved[2] = (retrieved[2]+retrieved[6]+retrieved[10]+retrieved[14])/4;


	brushes[selectedbrush].r = retrieved[0];
	brushes[selectedbrush].g = retrieved[1];
	brushes[selectedbrush].b = retrieved[2];
	brushes[selectedbrush].a = 0.5;//retrieved[3];
//printf("sampled\n");
}

