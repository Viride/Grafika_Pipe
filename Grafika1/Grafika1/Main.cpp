#include <gl/glew.h>
#include <gl/freeglut.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <vector>
#include<fstream>
#include "colors.h"
#include "materials.h"
#include "shaderprogram.h"
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "myCube.h"
// wskaŸnik na funkcjê glWindowPos2i

float speed = 0; // [radiany/s]
float observerx = 0.0;
float observery = 0.0;
float observerz = -40.0;
float height = 768;
float width = 1366;

struct Pola {
	int number;				//numer elementu 0-80
	float x_left, x_right;	// wspó³rzêdne do kwadratu
	float y_up, y_down;		// wspó³rzêdne do kwadratu
	int pozycja;			// u³o¿enie elementu 1-4 (tak jak jest u³o¿ony)
};
Pola tab[81];

void inicjalizacja(Pola tab[]) {

	int licz = 0;
	float x = -13.5;
	float y = 13.5;
	int zmiana = 3; // o tyle s¹ przesuwane elementy

	for (int i = 0;i < 9;i++) {
		for (int j = 0;j < 9;j++) {
			tab[j + 9 * i].y_up = y;
			tab[j + 9 * i].y_down = y - 3.0;
			tab[j + 9 * i].number = j + 9 * i;
		}
		y -= zmiana;
	}

	for (int i = 0;i < 9;i++) {
		for (int j = 0;j < 9;j++) {
			tab[i + 9 * j].x_left = x;
			tab[i + 9 * j].x_right = x + 3.0;
		}
		x += zmiana;
	}

}


void key_callback(GLFWwindow* window, int key,
	int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) observerx -= 1.0;
		if (key == GLFW_KEY_RIGHT) observerx += 1.0;
		if (key == GLFW_KEY_UP) observery -= 1.0;
		if (key == GLFW_KEY_DOWN) observery += 1.0;
		if (key == GLFW_KEY_LEFT_BRACKET) observerz -= 1.0;
		if (key == GLFW_KEY_RIGHT_BRACKET) observerz += 1.0;
	}

	if (action == GLFW_RELEASE) {
		speed = 0;
	}
}

//Procedura obs³ugi b³êdów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura inicjuj¹ca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który nale¿y wykonaæ raz, na pocz¹tku programu************
	glClearColor(1, 1, 1, 1); //Czyœæ ekran na czarno
							  //glEnable(GL_LIGHTING); //W³¹cz tryb cieniowania
	glEnable(GL_LIGHT0); //W³¹cz domyslne œwiat³o
	glEnable(GL_DEPTH_TEST); //W³¹cz u¿ywanie Z-Bufora
	glEnable(GL_COLOR_MATERIAL); //glColor3d ma modyfikowaæ w³asnoœci materia³u

	glfwSetKeyCallback(window, key_callback);

}

//Procedura rysuj¹ca zawartoœæ sceny
void drawScene(GLFWwindow* window, float angle) {
	//************Tutaj umieszczaj kod rysuj¹cy obraz******************l

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wykonaj czyszczenie bufora kolorów


	glm::mat4 V = glm::lookAt( //Wylicz macierz widoku
		glm::vec3(0.0f, 0.0f, observerz),
		glm::vec3(observerx, observery, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 P = glm::perspective(45 * PI / 180, width/height, 1.0f, 150.0f); //Wylicz macierz rzutowania

																	  //Za³aduj macierz rzutowania do OpenGL
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(P));

	//PrzejdŸ w tryb pracy z macierz¹ Model-Widok
	glMatrixMode(GL_MODELVIEW);

	//Wylicz macierz obrotu o k¹t angle wokó³ osi (0,1,1)
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::rotate(M, angle, glm::vec3(0, 1, 1));
	glLoadMatrixf(glm::value_ptr(V*M)); //Za³aduj wyliczon¹ macierz do OpenGL


										//Narysuj model
	glEnableClientState(GL_VERTEX_ARRAY); //W³¹cz u¿ywanie tablicy wierzcho³ków
	glEnableClientState(GL_COLOR_ARRAY); //W³¹cz u¿ywanie tablicy kolorów

	//glVertexPointer(3, GL_FLOAT, 0, myCubeVertices); //Wska¿ tablicê wierzcho³ków
	//glColorPointer(3, GL_FLOAT, 0, myCubeColors); //Wska¿ tablicê kolorów

	//glDrawArrays(GL_QUADS, 0, myCubeVertexCount); //Wykonaj rysowanie
	glColor3f(1.0, 0, 0);
	glRectf(1.0, 1.0, -1.0, -1.0);
	for (int i = 0;i < 81;i++) {
		if(i%2==0) glColor3f(1.0, 0, 0);
		else glColor3f(0, 1.0, 0);
		glRectf(tab[i].x_left, tab[i].y_up, tab[i].x_right, tab[i].y_down);
	}

	glDisableClientState(GL_VERTEX_ARRAY); //Wy³¹cz u¿ywanie tablicy wierzcho³ków
	glDisableClientState(GL_COLOR_ARRAY); //Wy³¹cz u¿ywanie tablicy kolorów

										  //Przerzuæ tylny bufor na przedni
	glfwSwapBuffers(window);

}

int main(void)
{
	inicjalizacja(tab);
	for (int i = 0; i < 81; i++) {
		std::cout << tab[i].x_left << " " << tab[i].x_right << " " << tab[i].y_down << " " << tab[i].y_up << std::endl;
	}
	GLFWwindow* window; //WskaŸnik na obiekt reprezentuj¹cy okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurê obs³ugi b³êdów

	if (!glfwInit()) { //Zainicjuj bibliotekê GLFW
		fprintf(stderr, "Nie mo¿na zainicjowaæ GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(width, height, "Pipe", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL. 
	if (!window) //Je¿eli okna nie uda³o siê utworzyæ, to zamknij program
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje siê aktywny i polecenia OpenGL bêd¹ dotyczyæ w³aœnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekê GLEW
		fprintf(stderr, "Nie mo¿na zainicjowaæ GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjuj¹ce

	float angle = 0; //K¹t obrotu torusa
	glfwSetTime(0); //Wyzeruj licznik czasu
	//glutReshapeFunc(Reshape);

					//G³ówna pêtla
	while (!glfwWindowShouldClose(window)) //Tak d³ugo jak okno nie powinno zostaæ zamkniête
	{
		drawScene(window, angle); //Wykonaj procedurê rysuj¹c¹
		glfwPollEvents(); //Wykonaj procedury callback w zaleznoœci od zdarzeñ jakie zasz³y.
	}

	glfwDestroyWindow(window); //Usuñ kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajête przez GLFW
	exit(EXIT_SUCCESS);
}
/*
using namespace Core;

GLuint program;

void renderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0, 0.0, 0.0, 1.0);//clear red

									 //use the created program
	glUseProgram(program);

	//draw 3 vertices as triangles
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glutSwapBuffers();
}

void Init()
{

	glEnable(GL_DEPTH_TEST);

	//load and compile shaders
	Core::Shader_Loader shaderLoader;
	program = shaderLoader.CreateProgram("Vertex_Shader.glsl",
		"Fragment_Shader.glsl");
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Drawing my first triangle");
	glewInit();

	Init();

	// register callbacks
	glutDisplayFunc(renderScene);
	glutMainLoop();
	glDeleteProgram(program);
	return 0;

}
*/