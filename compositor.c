

//with 512mb we can theoretically store 61x1024 or 15x2048 or 3x4096
//with 1gb we can theoretically store 119x1024 or 29x2048 or 7x4096 or 1x8192
//quadruple that for 8 bit textures
//#define LAYERSIZE 1024
//#define LAYERSIZE 2048
#define LAYERSIZE 4096
//#define LAYERSIZE 8192
#define NUMLAYERS 2
#define NUMBRUSHES 11//why 11 and not 10?
#define PI 3.1415926
//#include "atexture.xbm"

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
	float density;
	char softedge;
	char erase;
	char scalepressure;
	char blur;
	char spin;
	char colorshifta;
	char constantsize;
} brush_t;
//typedef char layerdatasmall_t[LAYERSIZE][4];


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

static inline void clamp(float *value, float min, float max){*value = *value>max?max:*value<min?min:*value;}

static void target(GLuint texid, int resx, int resy)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);//switch fbos
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texid, 0);//attach texture
	int fbores = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(fbores != GL_FRAMEBUFFER_COMPLETE)
		printf("incomplete framebuffer %d\n", fbores);
	glViewport(0,0,resx,resy);
}
static void untarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);//switch to default fbo
	glViewport(0,0,windoww,windowh);
}

static void drawlayer(int ind)
{
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, layers[ind].texid);
	glBegin(GL_QUADS);
	glColor4f(1,1,1,layers[ind].opacity);
	glColor4f(layers[ind].opacity,layers[ind].opacity,layers[ind].opacity,layers[ind].opacity);
	float xt, yt;
	float rot = rotation;
	xt = -1*zoom;
	yt = -1*zoom;
	glTexCoord2f(0,0);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa +panx*zoom,cos(rot)*yt-sin(rot)*xt + pany*zoom,0);
	xt = -1*zoom;
	yt = 1*zoom;
	glTexCoord2f(0,1);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa + panx*zoom,cos(rot)*yt-sin(rot)*xt + pany*zoom,0);
	xt = 1*zoom;
	yt = 1*zoom;
	glTexCoord2f(1,1);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa + panx*zoom,cos(rot)*yt-sin(rot)*xt + pany*zoom,0);
	xt = 1*zoom;
	yt = -1*zoom;
	glTexCoord2f(1,0);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa + panx*zoom,cos(rot)*yt-sin(rot)*xt +pany*zoom,0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}
static void drawwheel()
{
	glBindTexture(GL_TEXTURE_2D, wheeltexture);
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1,1,1,1);
//	glColor4f(0.5,0.5,0.5,0.5);
//	glColor4f(brushes[selectedbrush].r+0.1,brushes[selectedbrush].g+0.1,brushes[selectedbrush].b+0.1,1);
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
static void wtol(float *x, float *y)
{
	*x = ((*x-0.5)*2/windowa)/zoom-panx/windowa;
	*y = ((*y-0.5)*-2)/zoom-pany;
	float tx = -*y, ty=*x;//don't fucking touch this shit
	float ay = sin(rotation)*ty - cos(rotation)*tx;
	float ax = sin(rotation)*tx + cos(rotation)*ty;
	*x = ax;
	*y = ay;
}
static GLuint loadtex(char *filename)
{
	GLuint rval;
	int i,j;
	glGenTextures(1, &rval);
	glBindTexture(GL_TEXTURE_2D, rval);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	FILE *f = fopen(filename, "r");
	unsigned char data[512*512*4+512];
	fread(data, 1, 512*512*4+512, f);
	unsigned char realdata[512*512*4];
	for(i=0;i<512;i++)
	{
		for(j=0;j<512;j++)
		{
			float alpha = ((float)data[(i*512+j) +3*512*512+ 512])/255.f;
			realdata[(i*512+j)*4 + 0] = alpha*data[(i*512+j) +0*512*512+ 512];//*alpha;
			realdata[(i*512+j)*4 + 1] = alpha*data[(i*512+j) +1*512*512+ 512];//*alpha;
			realdata[(i*512+j)*4 + 2] = alpha*data[(i*512+j) +2*512*512+ 512];//*alpha;
			realdata[(i*512+j)*4 + 3] = alpha*255;//data[(i*512+j) +3*512*512+ 512];
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, realdata);
	return rval;
}
void c_init()
{
	int i, j;
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glGenFramebuffers(1,&fbo);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
//	glClampColorARB(0x891B, GL_TRUE);//CLAMP_FRAGMENT_COLOR_ARB
	glClearColor(1,1,1,1);
	for(i=0;i<NUMLAYERS;i++)
	{
		glGenTextures(1,&layers[i].texid);
		glBindTexture(GL_TEXTURE_2D, layers[i].texid);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, LAYERSIZE, LAYERSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);//clear instead of uploading to not use lods of ram
		target(layers[i].texid, LAYERSIZE, LAYERSIZE);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0,0,0,0);
		untarget();
		layers[i].opacity = 1;
	}
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
				for(k=0;k<3;k++)if(brightness>0.5)serpiw[i][j][k]+=brightness-0.5;else serpiw[i][j][k]*=brightness*2;
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
		brushes[i].a= 0.75;
		brushes[i].texid = 0;
		brushes[i].density = 100;
	}
	brushes[0].r=brushes[0].g=brushes[0].b=0.5;
	brushes[1].scalepressure = 1;
	brushes[1].size = 2;
	brushes[1].r = 0.5;
	brushes[2].g = 0.5;
	brushes[2].softedge = 1;
	brushes[3].erase = 1;
	brushes[4].blur = 1;
//	brushes[4].softedge= 1;
	brushes[4].a= 0.1;

	brushes[5].texid = loadtex("atexture.rgba");
	brushes[5].spin = 1;
	brushes[5].r= 1;
	brushes[5].g= 1;
	brushes[5].b= 1;
	brushes[5].a= 0.1;
	brushes[6] = brushes[5];
	brushes[6].blur = 1;
	brushes[6].a= 0.4;
	brushes[7] = brushes[5];
	brushes[7].colorshifta = 1;
	brushes[7].b = 0.2;
	brushes[7].g = 0.5;
	brushes[7].a= 1.8;
	brushes[8].constantsize = 1;
	brushes[8].texid = loadtex("anothertexture.rgba");
	brushes[9].texid = loadtex("gimpgalaxybig.rgba");
	brushes[9].density = 500;
	brushes[9].a = 0.5;
	brushes[9].scalepressure = 1;
}

void c_draw()
{
//	target(layers[1].texid);
//	drawlayer(0);
	untarget();
	int i;
	for(i=0;i<NUMLAYERS;i++)
		drawlayer(i);
	glLineStipple(1, 0xFFFF);
	glBegin(GL_LINE_STRIP);
	float colors[/*NUMLAYERS*/4*3] = {1,1,0, 0,1,1, 1,0,0, 0,1,0};//must be at least NUMLAYERS*3, but not doing that since NUMLAYERS can be less than 3
	float xt,yt;
	float rot = rotation;
	glColor4f(colors[3*currentlayer],colors[3*currentlayer+1],colors[3*currentlayer+2],1);
	xt = -1*zoom;
	yt = -1*zoom;
	glTexCoord2f(0,0);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa +panx*zoom,cos(rot)*yt-sin(rot)*xt + pany*zoom,0);
	xt = -1*zoom;
	yt = 1*zoom;
	glTexCoord2f(0,1);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa + panx*zoom,cos(rot)*yt-sin(rot)*xt + pany*zoom,0);
	xt = 1*zoom;
	yt = 1*zoom;
	glTexCoord2f(1,1);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa + panx*zoom,cos(rot)*yt-sin(rot)*xt + pany*zoom,0);
	xt = 1*zoom;
	yt = -1*zoom;
	glTexCoord2f(1,0);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa + panx*zoom,cos(rot)*yt-sin(rot)*xt +pany*zoom,0);
	xt = -1*zoom;
	yt = -1*zoom;
	glTexCoord2f(0,0);glVertex3f((cos(rot)*xt+sin(rot)*yt)*windowa +panx*zoom,cos(rot)*yt-sin(rot)*xt + pany*zoom,0);
	glEnd();
	if(renderwheel)
		drawwheel();
	GLuint err = glGetError();
	if(err != 0)
		printf("gl error %x\n", err);
}

void c_selectlayer(int lay)
{
	currentlayer = lay % NUMLAYERS;
}
void c_windowsize(float x, float y)
{
	windoww = x;
	windowh = y;
	windowa = y/x;
}
void c_layeropacity(float change)
{
	layers[currentlayer].opacity = (layers[currentlayer].opacity + change > 0)?((layers[currentlayer].opacity + change <= 1)?layers[currentlayer].opacity + change:1 ):0;
	//FIXME:have to multiply layer data by newalpha/oldalpha
}
void b_color(float r, float g, float b, float a)
{
	brushes[selectedbrush].r = r;
	brushes[selectedbrush].g = g;
	brushes[selectedbrush].b = b;
	if(a >= 0)
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
//TODO:fix premultiplied alpha
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
//	panx *= zto/zoom;
//	pany *= zto/zoom;
	zoom = zto;
//	panx = panx*zoom>1?1:panx*zoom<-1?-1:panx;
//	pany = pany*zoom>1?1:pany*zoom<-1?-1:pany;
	clamp(&panx, -1/zoom - 1, 1/zoom + 1);
	clamp(&pany, -1/zoom - 1, 1/zoom + 1);
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
	panx += dx*2/zoom;
	pany -= dy*2/zoom;
	clamp(&panx, -1/zoom - 1, 1/zoom + 1);
	clamp(&pany, -1/zoom - 1, 1/zoom + 1);
}
void c_rotate(float dtheta)
{
	rotation += dtheta;
}
void c_rotation(float theta)
{
	rotation = theta;
}
void c_droppersample(float x, float y)
{
//	c_draw();//redraw to not catch old cursors etc FIXME:this doesn't work and is slow as shit
	untarget();
	float retrieved[4*4];
	glReadPixels(x*windoww, (1-y)*windowh, 2,2,GL_RGBA, GL_FLOAT, retrieved);
	retrieved[0] = (retrieved[0]+retrieved[4]+retrieved[8]+retrieved[12])/4;
	retrieved[1] = (retrieved[1]+retrieved[5]+retrieved[9]+retrieved[13])/4;
	retrieved[2] = (retrieved[2]+retrieved[6]+retrieved[10]+retrieved[14])/4;
	retrieved[3] = (retrieved[3]+retrieved[7]+retrieved[11]+retrieved[15])/4;
//	if(retrieved[3] == 0)
//		return;


	brushes[selectedbrush].r = retrieved[0];
	brushes[selectedbrush].g = retrieved[1];
	brushes[selectedbrush].b = retrieved[2];
}

static void paint(float x, float y, float pressure)//TODO:replace the public version of this, move it with the other private ones
{
	glBindTexture(GL_TEXTURE_2D,brushes[selectedbrush].texid);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	if(brushes[selectedbrush].erase)
		glBlendFuncSeparate(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
	//	glBlendFunc(GL_ZERO, GL_ZERO);
	
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.5,0.5);
	float a = pressure*brushes[selectedbrush].a/**((((float)rand())/RAND_MAX - 0.5)*0.1 + 1)*/;
	if(!brushes[selectedbrush].colorshifta)
		clamp(&a,0,1);
//	if(brushes[selectedbrush].erase)
//		glColor4f(0,0,0, 0);//blending doesn't work so good with erasing//FIXME:make this work with premultiplied alpha
//	else
	glColor4f(brushes[selectedbrush].r*a, brushes[selectedbrush].g*a,brushes[selectedbrush].b*a, a);
	glVertex3f(x     , y, 0);
	if(brushes[selectedbrush].softedge)
		a = 0;
//	if(!brushes[selectedbrush].erase)
	glColor4f(brushes[selectedbrush].r*a, brushes[selectedbrush].g*a,brushes[selectedbrush].b*a, a);
	float spinoffs;
	if(brushes[selectedbrush].spin)
		spinoffs = rand()/(float)RAND_MAX*PI*2;
	int j;
	float brushsize = 0.1*brushes[selectedbrush].size;
	if(brushes[selectedbrush].scalepressure)
		brushsize *= pressure;
	if(!brushes[selectedbrush].constantsize)
		brushsize /= zoom;
	for(j=0;j<51;j++)
	{
		glTexCoord2f(sin(j*PI/25+spinoffs + rotation)*0.5+0.5     , cos(j*PI/25+spinoffs + rotation)*0.5+0.5);
		glVertex3f(x  +sin(j*PI/25)*brushsize     , y + cos(j*PI/25)*brushsize, 0);
	}

	glEnd();
}
void c_paintline(float startx, float starty, float startpressure, float endx, float endy, float endpressure)
{
	float samplex = startx, sampley = starty;
	wtol(&startx, &starty);
	wtol(&endx, &endy);
	if(brushes[selectedbrush].blur && abs(startx)<1 && abs(starty)<1)
		c_droppersample(samplex,sampley);
	target(layers[currentlayer].texid, LAYERSIZE, LAYERSIZE);

	
	int numsamples = ceil(sqrt(pow(startx-endx, 2) + pow(starty-endy, 2))*brushes[selectedbrush].density);
	int i;
	for(i=0;i<numsamples;i++)
		paint((startx*i + endx*(numsamples-i))/numsamples,(starty*i + endy*(numsamples-i))/numsamples,(startpressure*i + endpressure*(numsamples-i))/numsamples);
	untarget();
}
void c_viscursor(float x, float y )
{

	//glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	//glBlendFunc(GL_ONE, GL_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//	wtos(&x, &y);
	x = (x*2)-1;
	y = (y*-2)+1;
	glBindTexture(GL_TEXTURE_2D,0);//disable texturing
	glEnable(GL_LINE_STIPPLE);
	if(brushes[selectedbrush].erase)
	{
		glColor4f(1, 1,0.5, 0);
		glLineStipple(1, 0xCCCC);
	}
	else if(brushes[selectedbrush].blur)
	{

		glColor4f(0.5, 0.5,0.5, 1);
		glLineStipple(1, 0x00F0);
	}
	else
	{
		glColor4f(brushes[selectedbrush].r, brushes[selectedbrush].g,brushes[selectedbrush].b, 1);
		glLineStipple(1, 0xFFFF);
	}
//	glBlendFunc(GL_ONE, GL_ONE);
	if(brushes[selectedbrush].texid == 0 || brushes[selectedbrush].blur || renderwheel)//TODO:maybe a better way to do this?  whatever
		glBegin(GL_LINE_STRIP);
	else
	{
		glBindTexture(GL_TEXTURE_2D, brushes[selectedbrush].texid);
		glBegin(GL_TRIANGLE_FAN);
		glTexCoord2f(0.5,0.5);
		glVertex3f(x,y,0);
	}
	int j;
	for(j=0;j<21;j++)
	{
		//glTexCoord2f(sin(j*PI/10)*0.5+0.5     , cos(j*PI/10)*0.5+0.5);
		glTexCoord2f(sin(j*PI/10)*0.5+0.5     , cos(j*PI/10)*0.5+0.5);
		if(brushes[selectedbrush].constantsize)
			glVertex3f(x  +0.1*sin(j*PI/10)*windowa*brushes[selectedbrush].size*zoom, y + 0.1*cos(j*PI/10)*brushes[selectedbrush].size*zoom, 0);
		else
			glVertex3f(x  +0.1*sin(j*PI/10)*windowa*brushes[selectedbrush].size, y + 0.1*cos(j*PI/10)*brushes[selectedbrush].size, 0);
/*		if(brushes[selectedbrush].texid != 0)//draw an inner circle if we're visualizing a texture, so point sampling works
		{
			glTexCoord2f(sin(j*PI/10)*0.05+0.5     , cos(j*PI/10)*0.05+0.5);
			glVertex3f(x  +0.01*sin(j*PI/10)*windowa*brushes[selectedbrush].size, y + 0.01*cos(j*PI/10)*brushes[selectedbrush].size, 0);
		}*/
	}
	glEnd();
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
}

