#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct BMPImagem
{
	int width;
	int height;
	char *data;
} BMPImage;

#define MAX_NO_TEXTURES 8
#define M_PI 3.14159265358979323846264338327

GLfloat angleX = 0.0f;
GLfloat angleY = 0.0f;

int chave1=0, chave2=0, chave3=0;
int cubesize = 2;
float camerapos[3] = {-8.0, 0.0, -38.0};
float camerarot = 0.0;
int rotate_angle = 5;
float collision_padding = 0.05;

GLuint texture_id[MAX_NO_TEXTURES];

char *filenameArray[MAX_NO_TEXTURES] = {
	"textura/parede1.bmp",
	"textura/chao.bmp",
	"textura/teto.bmp",
	"textura/block.bmp",
	"textura/letraN/N.bmp",
	"textura/letraN/S.bmp",
	"textura/unifesp.bmp",
	"textura/quadro.bmp"};
/*Labirinto é feito com essa matriz de 16 por 16, onde 1 fica a parede, 
  0 os espaços vazios 4 é a porta de saida, 2 e 3 se encontram as letras.*/

int maze[16][16] = {
	{1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1},
	{1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1},
	{1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1},
	{1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1},
	{1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1},
	{1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1},
	{1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 3, 1},
	{1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1},
	{1, 1, 0, 1, 0, 1, 0, 1, 2, 1, 0, 1, 0, 1, 0, 1},
	{1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

void getBitmapImageData(char *pFileName, BMPImage *pImage)
{
	FILE *pFile = NULL;
	unsigned short nNumPlanes;
	unsigned short nNumBPP;
	int i;

	if ((pFile = fopen(pFileName, "rb")) == NULL)
		printf("ERROR: getBitmapImageData - %s not found.\n", pFileName);

	// Seek forward to width and height info
	fseek(pFile, 18, SEEK_CUR);

	if ((i = fread(&pImage->width, 4, 1, pFile)) != 1)
		printf("ERROR: getBitmapImageData - Couldn't read width from %s.\n ", pFileName);

	if ((i = fread(&pImage->height, 4, 1, pFile)) != 1)
		printf("ERROR: getBitmapImageData - Couldn't read height from %s.\n ", pFileName);

	if ((fread(&nNumPlanes, 2, 1, pFile)) != 1)
		printf("ERROR: getBitmapImageData - Couldn't read plane count from %s.\n", pFileName);

	if (nNumPlanes != 1)
		printf("ERROR: getBitmapImageData - Plane count from %s.\n ", pFileName);

	if ((i = fread(&nNumBPP, 2, 1, pFile)) != 1)
		printf("ERROR: getBitmapImageData - Couldn't read BPP from %s.\n ", pFileName);

	if (nNumBPP != 24)
		printf("ERROR: getBitmapImageData - BPP from %s.\n ", pFileName);

	// Seek forward to image data
	fseek(pFile, 24, SEEK_CUR);

	// Calculate the image's total size in bytes. Note how we multiply the
	// result of (width * height) by 3. This is becuase a 24 bit color BMP
	// file will give you 3 bytes per pixel.
	int nTotalImagesize = (pImage->width * pImage->height) * 3;

	pImage->data = (char *)malloc(nTotalImagesize);

	if ((i = fread(pImage->data, nTotalImagesize, 1, pFile)) != 1)
		printf("ERROR: getBitmapImageData - Couldn't read image data from %s.\n ", pFileName);

	//
	// Finally, rearrange BGR to RGB
	//

	char charTemp;
	for (i = 0; i < nTotalImagesize; i += 3)
	{
		charTemp = pImage->data[i];
		pImage->data[i] = pImage->data[i + 2];
		pImage->data[i + 2] = charTemp;
	}
}

void CarregaTexturas()
{
	BMPImage textura;

	glGenTextures(MAX_NO_TEXTURES, texture_id);

	int i;
	for (i = 0; i < MAX_NO_TEXTURES; i++)
	{
		getBitmapImageData(filenameArray[i], &textura);
		glBindTexture(GL_TEXTURE_2D, texture_id[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, textura.width, textura.height, 0, GL_RGB, GL_UNSIGNED_BYTE, textura.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}

void initTexture(void)
{
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	CarregaTexturas();
}

void lighting()
{
	GLfloat light0_pos[] = {2.0f, 2.0f, 2.0f, 1.0f};
	GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat black[] = {0.0f, 0.0f, 0.0f, 1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, black);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	// Atenuação radial
	// glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.5f);   //define a0
	// glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.15f);    //define a1
	// glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.1f);  //define a2

	// Fonte de Luz Direcional - Efeito de Holofote
	GLfloat light1_pos[] = {-2.0f, 0.0f, 0.0f, 1.0f};
	glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT1, GL_SPECULAR, white);
	GLfloat direction[] = {1.0f, 0.0f, 0.0f};
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction); // vetor direção
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.0f);			// espalhamento angular
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 0.1f);		// atenuação angular

	// Parâmetros definidos globalmente
	// GLfloat global_ambient[] = {0.9f, 0.9f, 0.9f, 1.0f};
	// glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
	////Reflexão especular definida como a posição de visão corrente
	// glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
	////Habilitar cálculos de iluminação para ambas as faces dos polígonos
	// glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_COLOR_MATERIAL);
}

double convert_to_rad(double angle_in_deg)
{
	double angle_in_rad;
	angle_in_rad = (angle_in_deg * M_PI) / 180;
	return angle_in_rad;
}

float *getIntendedPosition(float camera_rot, float camera_x, float camera_z, float local_angle, float modifier)
{

	float step_distance = 0.2;
	float *position = (float *)malloc(3 * sizeof(float));
	position[0] = camera_x;
	position[1] = 0;
	position[2] = camera_z;

	float x_offset = step_distance * cos(convert_to_rad(camera_rot + local_angle));
	float z_offset = step_distance * sin(convert_to_rad(camera_rot + local_angle));

	position[0] += (x_offset * modifier);
	position[2] += (z_offset * modifier);

	return position;
}

//FUNÇÃO USADA PARA CRIAR O CHÃO DO JOGO
void drawPlane()
{

	glEnable(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture(GL_TEXTURE_2D, 0);

	//glBindTexture(GL_TEXTURE_2D, texture_id[1]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0, 1.0, -1.0);
	glTexCoord2f(texture_id[1], 0.0);
	glVertex3f(-1.0, 1.0, -1.0);
	glTexCoord2f(texture_id[1], texture_id[1]);
	glVertex3f(-1.0, 1.0, 1.0);
	glTexCoord2f(0.0, texture_id[1]);
	glVertex3f(1.0, 1.0, 1.0);
	glEnd();
}

//CRIA AS PAREDES DO LABIRINTO USANDO CUBOS, VARIAVEL TEXTURE PARA MUDAR A TEXTURA
void drawCube(int texture)
{
	float texture_tile = 1.0;

	glEnable(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glBindTexture(GL_TEXTURE_2D, texture_id[texture]);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, -1.0, 1.0);
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(1.0, -1.0, 1.0);
	glTexCoord2f(texture_tile, texture_tile);
	glVertex3f(1.0, 1.0, 1.0);
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(-1.0, 1.0, 1.0);
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(-1.0, -1.0, -1.0);
	glTexCoord2f(texture_tile, texture_tile);
	glVertex3f(-1.0, 1.0, -1.0);
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(1.0, 1.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0, -1.0, -1.0);
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(-1.0, 1.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, 1.0, 1.0);
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(1.0, 1.0, 1.0);
	glTexCoord2f(texture_tile, texture_tile);
	glVertex3f(1.0, 1.0, -1.0);
	glTexCoord2f(texture_tile, texture_tile);
	glVertex3f(-1.0, -1.0, -1.0);
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(1.0, -1.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0, -1.0, 1.0);
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(-1.0, -1.0, 1.0);
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(1.0, -1.0, -1.0);
	glTexCoord2f(texture_tile, texture_tile);
	glVertex3f(1.0, 1.0, -1.0);
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(1.0, 1.0, 1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0, -1.0, 1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, -1.0, -1.0);
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(-1.0, -1.0, 1.0);
	glTexCoord2f(texture_tile, texture_tile);
	glVertex3f(-1.0, 1.0, 1.0);
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(-1.0, 1.0, -1.0);

	glEnd();
}

//CRIA UM RETANGULO COM IMAGEM DA UNIFESP
void drawRectangle() {
	glBindTexture(GL_TEXTURE_2D, texture_id[6]);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); 
	glVertex2f(-4.0, -0.5);  // Bottom-left vertex
	glTexCoord2f(1.0, 0.0); 
	glVertex2f(30.0, -0.5);  // Bottom-right vertex
	glTexCoord2f(1.0, 1.0); 
	glVertex2f(30.0, 8.0);   // Top-right vertex
	glTexCoord2f(0.0, 1.0); 
	glVertex2f(-4.0, 8.0);   // Top-left vertex
    glEnd();
}
//cria um quadrado com a imagem de um quadro
void drawQuadro() {
	glBindTexture(GL_TEXTURE_2D, texture_id[7]);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); 
	glVertex2f(-2.0, -1.0);  // Bottom-left vertex
	glTexCoord2f(1.0, 0.0); 
	glVertex2f(2.0, -1.0);  // Bottom-right vertex
	glTexCoord2f(1.0, 1.0); 
	glVertex2f(2.0, 1.0);   // Top-right vertex
	glTexCoord2f(0.0, 1.0); 
	glVertex2f(-2.0, 1.0);   // Top-left vertex
    glEnd();
}

//FUNÇÃO PARA CRIAR A LETRA CUBO, Variavel Texture controla a imagem para a textura:
void drawCubeLetter(int texture)
{
	float texture_tile = 1.0;

	glEnable(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glBindTexture(GL_TEXTURE_2D, texture_id[texture]);

	glBegin(GL_QUADS);

	// Face Frontal
	
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.4, -0.4, 0.4);
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(0.4, -0.4, 0.4);
	glTexCoord2f(texture_tile, texture_tile);
	glVertex3f(0.4, 0.4, 0.4);
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(-0.4, 0.4, 0.4);
	

	// Face traseira
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(-0.4, -0.4, -0.4);
	glTexCoord2f(texture_tile, texture_tile);
	glVertex3f(-0.4, 0.4, -0.4);
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(0.4, 0.4, -0.4);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.4, -0.4, -0.4);

	// Face superior
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(-0.4, 0.4, -0.4);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.4, 0.4, 0.4);
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(0.4, 0.4, 0.4);
	glTexCoord2f(texture_tile, texture_tile);
	glVertex3f(0.4, 0.4, -0.4);

	// Face inferior
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(-0.4, -0.4, -0.4);
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(0.4, -0.4, -0.4);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.4, -0.4, 0.4);
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(-0.4, -0.4, 0.4);

	// Face direita
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(0.4, -0.4, -0.4);
	glTexCoord2f(texture_tile, texture_tile);
	glVertex3f(0.4, 0.4, -0.4);
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(0.4, 0.4, 0.4);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.4, -0.4, 0.4);

	// Face esquerda
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.4, -0.4, -0.4);
	glTexCoord2f(texture_tile, 0.0);
	glVertex3f(-0.4, -0.4, 0.4);
	glTexCoord2f(texture_tile, texture_tile);
	glVertex3f(-0.4, 0.4, 0.4);
	glTexCoord2f(0.0, texture_tile);
	glVertex3f(-0.4, 0.4, -0.4);


	glEnd();
}

//FUNÇÃO PARA SIMULAR A COLISÃO
//BASICAMENTE CRIA UMA HITBOX QUE SIMULA A BARREIRA DOS OBJETOS, E FAZ A VERIFICAÇÃ0
//COM AS COORDENAS X E Z PASSADAS COMO PARAMETRO
int colisao(int cube_size, int map[16][16], float x, float z, float padding, int op)
{

	float wall_x = 0.0;
	float wall_z = 0.0;

	int row_count = 0;
	int column_count = 0;

	float hitbox_size = ((cube_size / 2) + padding);

	for (int i = 0; i < 16; i++)
	{

		wall_z = (row_count * (cube_size * -1));

		for (int j = 0; j < 16; j++)
		{
			if (map[i][j] != op)
			{
				column_count += 1;
				continue;
			}
			wall_x = (column_count * (cube_size * -1));

			if ((z == wall_z) || ((z > (wall_z - hitbox_size)) && (z < (wall_z + hitbox_size))))
			{
				if ((x == wall_x) || ((x > (wall_x - hitbox_size)) && (x < (wall_x + hitbox_size))))
				{
					return 1;
				}
			}
			column_count += 1;
		}
		row_count += 1;
		column_count = 0;
	}

	return 0;
}
//função para fazer o controle da camera
void inputs(int key, int x, int y)
{
	if (key == GLUT_KEY_LEFT)
	{
		camerarot -= rotate_angle;
	}
	else if (key == GLUT_KEY_RIGHT)
	{
		camerarot += rotate_angle;
	}

	float pos_f[3] = {camerapos[0], 0, camerapos[2]};
	

	if ((key == GLUT_KEY_UP) || (key == GLUT_KEY_DOWN))
	{
		int modifier = (key == GLUT_KEY_UP) ? 1 : -1;
		float *aux_pos_f = getIntendedPosition(camerarot, camerapos[0], camerapos[2], 90, modifier);
		pos_f[0] = aux_pos_f[0];
		pos_f[1] = aux_pos_f[1];
		pos_f[2] = aux_pos_f[2];
	}


	float intended_x = pos_f[0];
	float intended_z = pos_f[2];
	//VERIFICA COLISAO
	//SE A NOVA POSIÇÃO DA CAMERA ESTIVER DEPOIS DA AREA DELIMITADA, NÃO SERA ATUALIZADA
	if (!colisao(cubesize, maze, intended_x, intended_z, collision_padding,1) && (!colisao(cubesize, maze, intended_x, intended_z, collision_padding,4)))
	{
		camerapos[0] = intended_x;
		camerapos[2] = intended_z;
	}
	
	//VERIFICA SE O PERSONGAGEM ENCONTROU A LETRA N
	if (colisao(cubesize, maze, intended_x, intended_z, 0.005,2)){
		maze[13][8] = 0;
		chave1 = 1;
	}
	//VERIFICA SE O PERSONGAGEM ENCONTROU A LETRA N
	if (colisao(cubesize, maze, intended_x, intended_z, 0.005,3)){
		maze[11][14] = 0;
		chave2 = 1;
	}
	//VERIFICA SE AS DUAS LETRAS FORAM ENCONTRADAS PARA ABRIR A PORTA DO LABIRINTOO
	if(chave1 & chave2 & maze[0][1]!=0){
		maze[0][1] = 0;
	}

	//VERIFICAÇÃO PARA APENAS CARREGAR A IMAGEM NO FINAL DO LABIRINTO
	if(camerapos[2]>-4 && camerapos[0]>-3){
		chave3 = 1;
	}
	

	glutPostRedisplay();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Visualização da câmera
	glTranslatef(0.0, 0.0, 0.0);
	glRotatef(camerarot, 0.0, 1.0, 0.0);
	glTranslatef(camerapos[0], camerapos[1], camerapos[2]);

	// Chão
	glPushMatrix();
	glTranslatef(0.0, -2.0, 0.0);
	glScalef(50.0, 1.0, 50.0);
	drawPlane();
	glPopMatrix();

	//Unifesp
	glPushMatrix();
	if(maze[0][1]==0 && chave3==1){
		glTranslatef(0,0,-20);
	}else{
		glTranslatef(0,-10,-20);
	}
	drawRectangle(6);
	glPopMatrix();

	//quadro
	glPushMatrix();
	glTranslatef(5,0,31.2);
	//glRotatef(90,0.0, 1.0, 0.0);
	drawQuadro();
	glPopMatrix();


	int row_count = 0;
	int column_count = 0;

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			if (maze[i][j] == 1)
			{
				drawCube(0);
			}
			if(maze[i][j]==2){
				drawCubeLetter(4);
			}
			if(maze[i][j]==3){
				drawCubeLetter(5);
			}
			if(maze[i][j]==4){
				drawCube(3);
			}
			glTranslatef(cubesize, 0.0, 0.0);
			column_count += 1;
		}
		glTranslatef(((cubesize * column_count) * -1), 0.0, cubesize);

		

		row_count += 1;
		column_count = 0;
	}



	glutSwapBuffers();
}

void init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 640.0f / 480.0f, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	lighting();
}

int main(int argc, char *argv[])
{
	//srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Labirinto Unifesp");
	glutSpecialFunc(inputs);
	// glutKeyboardFunc(keyboard);
	init();
	initTexture();
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}