
#define LAYERSIZE 2048
#define NUMLAYERS 16

typedef struct
{
	GLuint texid;
	float opacity;
} layer_t;

static layer_t layers[NUMLAYERS];
static GLuint fbo;


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
	glViewport(0,0,800,600);//TODO:make these right
}

static void drawlayer(int ind)
{
	glBindTexture(GL_TEXTURE_2D, layers[ind].texid);
	glBegin(GL_QUADS);
	glColor4f(1,1,1,layers[ind].opacity);
//	printf("op %d %f\n", ind, layers[ind].opacity);
	glTexCoord2f(0,0);glVertex3f(-1,-1,0);
	glTexCoord2f(0,1);glVertex3f(-1,1,0);
	glTexCoord2f(1,1);glVertex3f(1,1,0);
	glTexCoord2f(1,0);glVertex3f(1,-1,0);
	glEnd();
}
void c_init()
{
	int i, j;
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for(i=0;i<NUMLAYERS;i++)
	{
		glGenTextures(1,&layers[i].texid);
		glBindTexture(GL_TEXTURE_2D, layers[i].texid);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		layers[i].opacity = 0;
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, LAYERSIZE, LAYERSIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	glGenFramebuffers(1,&fbo);
	//make some more textures and shit here
//TODO:after this to be removed
	glBindTexture(GL_TEXTURE_2D, layers[0].texid);
	static float serpi[LAYERSIZE][LAYERSIZE][4];//causes stack overflow if not static
	for(i=0;i<LAYERSIZE;i++)
	{
		for(j=0;j<LAYERSIZE;j++)
		{
			serpi[i][j][0] = (j&i) == 0;
			serpi[i][j][3] = 1;
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, LAYERSIZE, LAYERSIZE, 0, GL_RGBA, GL_FLOAT, serpi);
	layers[0].opacity = 0.5;

	for(i=0;i<LAYERSIZE;i++)
	{
		for(j=0;j<LAYERSIZE;j++)
		{
			serpi[i][j][0]=0;
			serpi[i][j][1] = ((-j)&i)*0.0001;
			serpi[i][j][3] = ((-j)&i)*0.0001;
		}
	}
	glBindTexture(GL_TEXTURE_2D, layers[1].texid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, LAYERSIZE, LAYERSIZE, 0, GL_RGBA, GL_FLOAT, serpi);
	layers[1].opacity = 1;
	

}

void c_draw()
{
	static int sw =0;
	sw+= 0xffffff;
	target(layers[sw<0?1:0].texid);
	drawlayer(sw<0?0:1);
	untarget();
	int i;
	for(i=0;i<NUMLAYERS;i++)
		drawlayer(i);
	GLuint err = glGetError();
	if(err != 0)
		printf("gl error %d\n", err);
}

