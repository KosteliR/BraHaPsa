//#include <Windows.h>
//#include <GL\glew.h>
//#include <GL\freeglut.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "pch.h"
#include <math.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <GL/glut.h>

//lighting variables
float DiffuseLightWhite[] = { 1.0f, 1.0f ,0.875f };   //light for white cubes
float AmbientLightWhite[] = { 1.0, 1.0, 1.0 }; 
float LightPositionWhite[] = { 0.0, 1.0, 0.0 , 1.0 };
float DiffuseLight0[] = { 0.863f, 0.078f, 0.235f };   //light for red cubes
float AmbientLight0[] = { 0.863f, 0.078f, 0.235f }; 
float DiffuseLight1[] = { 0.0f, 0.0f, 0.804f };       //light for blue cubes
float AmbientLight1[] = { 0.0f, 0.0f, 0.804f }; 
float DiffuseLight2[] = { 0.000f, 1.000f, 0.498f };   //light for green cubes
float AmbientLight2[] = { 0.000f, 1.000f, 0.498f }; 
float DiffuseLight3[] = { 0.502f, 0.000f, 0.502f };   //light for purple cubes
float AmbientLight3[] = { 0.502f, 0.000f, 0.502f }; 
float DiffuseLight4[] = { 0.663f, 0.663f, 0.663f };   //light for gray cubes
float AmbientLight4[] = { 0.663f, 0.663f, 0.663f }; 
float LightPosition[] = { 1.0, 0.0, 0.0 , 1.0 };      //light position

//variables rquired for openGL
GLuint *textures;                               //the array for our textures
int mainMenu;
char scoreString[60];                           //printed score string

//window parameters
int width = 600, height = 600;                  

//variables for camera movement
float sqSize = 0.4, step=0.3, lastX, lastY;     
float x = 0.0, y = 0.0, z = 0.0;                //eye position
float cx = 0.0, cy = 0.0, cz = -1.0;            //center position
float ux = 0.0, uy = 1.0, uz = 0.0;             //up vector
float yaw = 0.0f, pitch = 0.0f;                 //angles for rotation
bool camLock = true, firstMouse = true;

//game logic variables
int gameState = 0, score;
int selX1 = 0, selY1 = 0, inputCount = 0;       //cube 1 selection (init required)
int	selX2, selY2;                               //cube 2 selection
int moveLimit;                                  //move limit variable
bool cubeMap[15][15];                        //holds which cubes exist
int colorMap[15][15];                           //holds the color of each cube
//colors held by colorMap
#define NOTHING -1
#define RED 0
#define BLUE 1
#define SCISSORS 2  //green
#define PAPER 3		//purple
#define ROCK 4		//gray
#define MOVELIMIT 30

// -----------------------------------
//            WINDOW SIZE
// -----------------------------------

void changeSize(int w, int h) {

	printf("screen size: %d,%d\n", w, h);
	width = w;
	height = h;
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	float ratio = w * 1.0 / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45.0f, ratio, 0.1f, 100.0f);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

// -----------------------------------
//           Load textures
// -----------------------------------

void loadAndMake(const char *filename, int i) {
	int width, height, nrChannels;
	glBindTexture(GL_TEXTURE_2D, textures[i]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

void loadTexturesFromFile() {
	glGenTextures(3, textures);
	loadAndMake("sci.jpg", 1);
	loadAndMake("paper.jpg", 2);
	loadAndMake("rock.jpg", 3);
}

void FreeTextures() {
	glDeleteTextures(3, textures);
}

// -----------------------------------
//          Make 3D objects
// -----------------------------------

void texSquare(int X, int Y, int i) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[i]);
	
	glBegin(GL_QUADS);
	glTexCoord2d(1.0, 1.0); glVertex3d(X - sqSize, Y - sqSize, -20.05);
	glTexCoord2d(0.0, 1.0); glVertex3d(X + sqSize, Y - sqSize, -20.05);
	glTexCoord2d(0.0, 0.0); glVertex3d(X + sqSize, Y + sqSize, -20.05);
	glTexCoord2d(1.0, 0.0); glVertex3d(X - sqSize, Y + sqSize, -20.05);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void makeCube(float x, float y, float r, float g, float b) {
	glPushMatrix();
	glColor3f(r, g, b);
	glTranslatef(x, y, -20.5);
	glutSolidCube(0.88);
	glPopMatrix();
}

void makeWhiteCubes() {
	glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseLightWhite); //change the light accordingly
	glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientLightWhite); //change the light accordingly
	glLightfv(GL_LIGHT0, GL_POSITION, LightPositionWhite); //change the light accordingly
	int i,j;
	for (i = 0; i < 15; i++) {
		for (j = 0; j < 15; j++) {
			makeCube((float)i-7, (float)j-7, 1.0f, 1.0f, 1.0f);
		}
	}
}

void makeColoredCubes() {
	int i, j;
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition); //change the light accordingly

	for (j = 0; j < 15; j++) {
		for (i = 0; i < 15; i++) {
			if (cubeMap[j][i]) {
				switch (colorMap[j][i]) {
					case 0:
						glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseLight0); //change the light accordingly
						glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientLight0); //change the light accordingly
						makeCube(j - 7.0, -i + 7.0, 0.863f, 0.078f, 0.235f);
						break;
					case 1:
						glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseLight1); //change the light accordingly
						glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientLight1); //change the light accordingly
						makeCube(j - 7.0, -i + 7.0, 0.0f, 0.0f, 0.804f);
						break;
					case 2:
						glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseLight2); //change the light accordingly
						glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientLight2); //change the light accordingly
						makeCube(j - 7.0, -i + 7.0, 0.000f, 1.000f, 0.498f);
						texSquare(j - 7.0, -i + 7.0, 1);
						break;
					case 3:
						glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseLight3); //change the light accordingly
						glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientLight3); //change the light accordingly
						makeCube(j - 7.0, -i + 7.0, 0.502f, 0.000f, 0.502f);
						texSquare(j - 7.0, -i + 7.0, 2);
						break;
					case 4:
						glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseLight4); //change the light accordingly
						glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientLight4); //change the light accordingly
						makeCube(j - 7.0, -i + 7.0, 0.663f, 0.663f, 0.663f);
						texSquare(j - 7.0, -i + 7.0, 3);
						break;
					default:
						break;
				}
			}
		}
	}
}

// -----------------------------------
//             DISPLAY
// -----------------------------------

void printString(char *str, int x, int y) {
	int i;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glRasterPos2i(x, y);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  //RGBA values of text color
	for(i=0; str[i]!='\0'; i++){
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
	glLoadIdentity();                 // Reset the model-view matrix
	gluLookAt(x, y, z,
			cx, cy, cz,
		    ux, uy, uz);

	glRotatef(pitch, 1, 0, 0);
	glRotatef(yaw, 0, 1, 0);
	glClearColor(0.0, 0.0, 0.0, 1.0); //clear the screen to black
	if (gameState == 0) {
		makeWhiteCubes();
	}
	if (gameState == 1) {
		sprintf(scoreString, "Score: %d   Moves left: %d", score, moveLimit);
		printString(scoreString, 10, height - 25);
		makeColoredCubes();
	}
	if (gameState == 2) {
		sprintf(scoreString, "Game Over! Score: %d", score);
		printString(scoreString, (width/2)-105, (height/2));
		sprintf(scoreString, "(right click -> start game)");
		printString(scoreString, (width/2)-105, (height/2)-30);
	}

	glEnd();
	glutSwapBuffers();  // Swap the front and back frame buffers (double buffering)
}

// -----------------------------------
//             KEYBOARD
// -----------------------------------

void processNormalKeys(unsigned char key, int xx, int yy) {
	//printf("%d pressed\n", key);
	if (key == 27) exit(0);
	else if (key == 114) {
		x = 0.0;
		y = 0.0;
		z = 0.0;
		cx = 0.0;
		cy = 0.0;
		cz = -1.0;
		ux = 0.0;
		uy = 1.0;
		uz = 0.0;
		pitch = 0.0;
		yaw = 0.0;
		firstMouse = true;
	}
	else if (key == 102) {
		printf("f\n");
		camLock = !camLock;
	}
}

void pressKey(int key, int xx, int yy) {
	switch (key) {
		case GLUT_KEY_LEFT:
			x -= step;
			cx -= step;
			break;
		case GLUT_KEY_RIGHT:
			x += step;
			cx += step;
			break;
		case GLUT_KEY_UP:
			y += step;
			cy += step;
			break;
		case GLUT_KEY_DOWN:
			y -= step;
			cy -= step;
			break;
		default:
			break;
	}
}

// -----------------------------------
//             MOUSE
// -----------------------------------

void cleanUp() {
	int i, j;
	for (j = 0; j < 15; j++) {
		for (i = 0; i < 15; i++) {
			if (!cubeMap[i][j]) {
				colorMap[i][j] = NOTHING;
			}
		}
	}
}

void destroyRange1(int X, int Y, int type) {
	if (X >= 0 && X <= 14 && Y >= 0 && Y <= 14) {
		//printf("1 checks %d,%d\n", X, Y);
		int thisType = colorMap[X][Y];                //type of neibour to destroy
		int oppsType;                                 //opposing type of neibour to destroy, opposing does not get destroyed

		if (type == 2) oppsType = 4;
		else oppsType = type - 1;
		//printf("range1 %d, %d is oppos\n", type, oppsType);
		if (thisType != oppsType) {                    //delete every neibour except opposing types
			cubeMap[X][Y] = false;
			//colorMap[X][Y] = NOTHING;
			score += 2;
		}
		else {
			score -= 1;
		}
	}
	return;
}

void destroyRange23(int X, int Y, int type) {
	if (X >= 0 && X <= 14 && Y >= 0 && Y <= 14) {
		//printf("2 checks %d,%d\n", X, Y);
		int thisType = colorMap[X][Y];
		int oppsType;                                //opposing type of neibour to destroy, opposing gets destroyed here!!
		if (type == 4) oppsType = 2;
		else oppsType = type + 1;
		//printf("range23 %d, %d is oppos\n", type, oppsType);
		if (thisType == oppsType) {                  //delete every opposing type neibour
			cubeMap[X][Y] = false;
			//colorMap[X][Y] = NOTHING;
			score += 3;
		}
	}
	return;
}

void explodeNeighbours(int midX, int midY, int type, int orientation) {
	printf("starting to explode for %d,%d\n", midX, midY);
	int i;

	if (orientation == 0) {
		midX -= 2;                                      //check range 1
		destroyRange1(midX, midY, type);
		midY--;
		for (i = 0; i < 4; i++) {
			destroyRange1(midX, midY, type);
			midX++;
		}
		destroyRange1(midX, midY, type);
		midY++;
		destroyRange1(midX, midY, type);
		midY++;
		for (i = 0; i < 5; i++) {
			destroyRange1(midX, midY, type);
			midX--;
		}                                              //end of check range 1

		midY++;										   //check range 2
		for (i = 0; i<4; i++) {
			destroyRange23(midX, midY, type);
			midY--;
		}
		for (i = 0; i<6; i++) {
			destroyRange23(midX, midY, type);
			midX++;
		}
		for (i = 0; i<4; i++) {
			destroyRange23(midX, midY, type);
			midY++;
		}
		for (i = 0; i<6; i++) {
			destroyRange23(midX, midY, type);
			midX--;
		}                                           //end of check range 2

		midX--;                                     //check range 3
		midY++;
		for (i = 0; i<6; i++) {
			destroyRange23(midX, midY, type);
			midY--;
		}
		for (i = 0; i<8; i++) {
			destroyRange23(midX, midY, type);
			midX++;
		}
		for (i = 0; i<6; i++) {
			destroyRange23(midX, midY, type);
			midY++;
		}
		for (i = 0; i<8; i++) {
			destroyRange23(midX, midY, type);
			midX--;
		}                                           //end of check range 3

	}
	else if (orientation = 1){
		midX--;                                    //check range 1
		midY += 2;
		for (i = 0; i < 4; i++) {
			destroyRange1(midX, midY, type);
			midY--;
		}

		destroyRange1(midX, midY, type);
		midX++;
		destroyRange1(midX, midY, type);
		midX++;

		for (i = 0; i < 4; i++) {
			destroyRange1(midX, midY, type);
			midY++;
		}

		destroyRange1(midX, midY, type);
		midX--;
		destroyRange1(midX, midY, type);       //end of check range 1

		midX -= 2;                             //check range 2
		midY++;

		for (i = 0; i < 6; i++) {
			destroyRange23(midX, midY, type);
			midY--;
		}
		for (i = 0; i < 4; i++) {
			destroyRange23(midX, midY, type);
			midX++;
		}
		for (i = 0; i < 6; i++) {
			destroyRange23(midX, midY, type);
			midY++;
		}
		for (i = 0; i < 4; i++) {
			destroyRange23(midX, midY, type);
			midX--;
		}                                      //end of check range 2

		midX--;                                //check range 3
		midY++;

		for (i = 0; i < 8; i++) {
			destroyRange23(midX, midY, type);
			midY--;
		}
		for (i = 0; i < 6; i++) {
			destroyRange23(midX, midY, type);
			midX++;
		}
		for (i = 0; i < 8; i++) {
			destroyRange23(midX, midY, type);
			midY++;
		}
		for (i = 0; i < 6; i++) {
			destroyRange23(midX, midY, type);
			midX--;
		}                                      //end of check range 3 
	}
	return;
}

void explodeCubes(int selX1, int selY1) {
	int i;
	int type;
	if (cubeMap[selX1][selY1]) {
		selY1--;
		for (i = 0; i < 3; i++) {
			if (selY1 - 1 >= 0 && selY1 + 1 <= 14) {
				if (colorMap[selX1][selY1] == colorMap[selX1][selY1 - 1] && colorMap[selX1][selY1] == colorMap[selX1][selY1 + 1]) {
					printf("%d,%d %d,%d %d,%d explode\n", selX1, selY1 - 1, selX1, selY1, selX1, selY1 + 1);
					type = colorMap[selX1][selY1];
					cubeMap[selX1][selY1] = false;
					cubeMap[selX1][selY1 - 1] = false;
					cubeMap[selX1][selY1 + 1] = false;
					colorMap[selX1][selY1] = NOTHING;
					colorMap[selX1][selY1 - 1] = NOTHING;
					colorMap[selX1][selY1 + 1] = NOTHING;
					score += 10;
					if (!(type == 0 || type == 1)) explodeNeighbours(selX1, selY1, type, 1);
					printf("score = %d\n", score);
				}
			}
			selY1++;
		}
		selY1 -= 2;
		selX1--;
		for (i = 0; i < 3; i++) {
			if (selX1-1>=0 && selX1+1<=14) {
				if (colorMap[selX1][selY1] == colorMap[selX1 - 1][selY1] && colorMap[selX1][selY1] == colorMap[selX1 + 1][selY1]) {
					printf("%d,%d %d,%d %d,%d explode\n", selX1 - 1, selY1, selX1, selY1, selX1 + 1, selY1);
					type = colorMap[selX1][selY1];
					cubeMap[selX1][selY1] = false;
					cubeMap[selX1 - 1][selY1] = false;
					cubeMap[selX1 + 1][selY1] = false;
					colorMap[selX1][selY1] = NOTHING;
					colorMap[selX1-1][selY1] = NOTHING;
					colorMap[selX1+1][selY1] = NOTHING;
					score += 10;
					if (!(type == 0 || type == 1)) explodeNeighbours(selX1, selY1, type, 0);
					printf("score = %d\n", score);
				}
			}
			selX1++;
		}
	}
}

void swapCubes(int selX1, int selY1, int selX2, int selY2) {
	printf("swapping %d,%d-> %d,%d\n", selX1, selY1, selX2, selY2);
	moveLimit--;
	if (moveLimit == 0) {
		gameState = 2;
		printf("game over!\n");
	}
	bool ex1 = cubeMap[selX1][selY1];
	bool ex2 = cubeMap[selX2][selY2];
	int temp;
	if (ex1 && !ex2) {
		cubeMap[selX2][selY2] = true;
		cubeMap[selX1][selY1] = false;
		colorMap[selX2][selY2] = colorMap[selX1][selY1];
		colorMap[selX1][selY1] = NOTHING;
		//printf("swapped1\n");
	}
	else if (!ex1 && ex2) {
		cubeMap[selX1][selY1] = true;
		cubeMap[selX2][selY2] = false;
		colorMap[selX1][selY1] = colorMap[selX2][selY2];
		colorMap[selX2][selY2] = NOTHING;
		//printf("swapped2\n");
	}
	else if (ex1 && ex2) {
		temp = colorMap[selX1][selY1];
		colorMap[selX1][selY1] = colorMap[selX2][selY2];
		colorMap[selX2][selY2] = temp;
		//printf("swapped3\n");
	}
	else {
		printf("both don't exist, wasted move\n");
	}
}

void mouseButton(int button, int state, int x, int y) {

	if (button == GLUT_LEFT_BUTTON) {
		int outX = (int)(0.36*width - 185.9);
		int outY = (int)(0.05*height - 3.09);
		int maxX = width - outX;
		int maxY = height - outY;
		int pxCubeX = (int)((width - 2 * outX) / 15);
		int pxCubeY = (int)((height - 2 * outY) / 15);

		if (state == GLUT_UP) {
			//printf("%d,%d\n", x, y);
			if (inputCount==0) {
				if (x >= outX && x <= maxX  && y >= outY && y <= maxY) {             //check that mouse has clicked inside the boxes
					inputCount = 1;
					selX1 = (x - outX) / pxCubeX;
					selY1 = (y - outY) / pxCubeY;
					//printf("selected1 %d,%d\n", selX1, selY1);
					return;
				}
			}
			if (inputCount == 1) {
				if (x >= outX && x <= maxX && y >= outY && y <= maxY) {             //check that mouse has clicked inside the boxes
					inputCount = 2;
					selX2 = (x - outX) / pxCubeX;
					selY2 = (y - outY) / pxCubeY;
					//printf("selected2 %d,%d\n", selX2, selY2);
					if ((abs(selX1 - selX2) <= 1 && (selY1 - selY2) == 0)||(abs(selY1 - selY2) <= 1 && (selX1 - selX2) == 0)) {
						swapCubes(selX1, selY1, selX2, selY2);
						explodeCubes(selX1, selY1);
						explodeCubes(selX2, selY2);
						cleanUp();
					}
				}
				inputCount = 0;
				return;
			}
		}
	}

}

void mouseMove(int x, int y) {
	if (!camLock) {
		if (firstMouse){
			lastX = x;
			lastY = y;
			firstMouse = false;
		}

		float xoffset = x - lastX;
		float yoffset = lastY - y;
		lastX = x;
		lastY = y;

		float sensitivity = 0.05;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}
}

// -----------------------------------
//             MENUS
// -----------------------------------

void processMainMenu(int status) {
	if (status == 0){
		int i, j;
		score = 0;
		moveLimit = MOVELIMIT;
		gameState = 1;
		for (j = 0; j < 15; j++) {
			for (i = 0; i < 15; i++) {
				cubeMap[i][j] = true;
				colorMap[i][j] = rand() % 5;       //make cube with random color 0-4
			}
		}
	}
	else
		exit(0);
}

// -----------------------------------
//             MAIN
// -----------------------------------

int main(int argc, char **argv) {

	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);			
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("BraXaPsa III");

	// register callbacks
	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(changeSize);

	//glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(pressKey);
	//glutSpecialUpFunc(releaseKey);
	glutMouseFunc(mouseButton);
	glutPassiveMotionFunc(mouseMove);


	// OpenGL init
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections
	glEnable(GL_DEPTH_TEST); //enable the depth testing
	glEnable(GL_LIGHTING); //enable the lighting
	glEnable(GL_LIGHT0); //enable LIGHT0, our Diffuse Light
	glEnable(GL_LIGHT1); //enable LIGHT1, our Ambient Light
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glShadeModel(GL_SMOOTH); //set the shader to smooth shader

	// init Menus
	mainMenu = glutCreateMenu(processMainMenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutAddMenuEntry("Start Game", 0);
	glutAddMenuEntry("Exit", 1);

	textures = new GLuint[3];
	loadTexturesFromFile();

	glutMainLoop();

	FreeTextures();
	return 0;
}
