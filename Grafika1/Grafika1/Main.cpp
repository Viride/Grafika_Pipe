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
// wska�nik na funkcj� glWindowPos2i

float speedx = 0; // [radiany/s]
float speedy = 0;
float observerx = 0.0;
float observery = 0.0;
float observerz = -40.0;
float height = 768;
float width = 1366;
int numer_pola=40;

struct Pola {
	int number;				//numer elementu 0-80
	float x_left, x_right;	// wsp�rz�dne do kwadratu
	float y_up, y_down;		// wsp�rz�dne do kwadratu
	int pozycja;			// u�o�enie elementu 1-4 (tak jak jest u�o�ony)
	int model;				// kt�ry element ma tam sta�
};
Pola tab[81];

void inicjalizacja(Pola tab[]) {

	int licz = 0;
	float x = -13.5;
	float y = 13.5;
	int zmiana = 3; // o tyle s� przesuwane elementy

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
		if (key == GLFW_KEY_K) speedx = -3.14;
		if (key == GLFW_KEY_I) speedx = 3.14;
		if (key == GLFW_KEY_J) speedy = -3.14;
		if (key == GLFW_KEY_L) speedy = 3.14;
		if (key == GLFW_KEY_W) {
			if (numer_pola > 8) numer_pola -= 9;
			else numer_pola = numer_pola + 81 - 9;
		}
		if (key == GLFW_KEY_S) {
			if (numer_pola < 72) numer_pola += 9;
			else numer_pola = numer_pola + 9 - 81;
		}
		if (key == GLFW_KEY_D) {
			if (numer_pola % 9 != 0) numer_pola -= 1;
			else numer_pola = numer_pola + 8;
		}
		if (key == GLFW_KEY_A) {
			if (numer_pola % 9 != 8) numer_pola += 1;
			else numer_pola = numer_pola - 8;
		}
	}

	if (action == GLFW_RELEASE) {
		speedy = 0;
		speedx = 0;
	}
}

//Procedura obs�ugi b��d�w
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura inicjuj�ca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, kt�ry nale�y wykona� raz, na pocz�tku programu************
	glClearColor(1, 1, 1, 1); //Czy�� ekran na czarno
							  //glEnable(GL_LIGHTING); //W��cz tryb cieniowania
	glEnable(GL_LIGHT0); //W��cz domyslne �wiat�o
	glEnable(GL_DEPTH_TEST); //W��cz u�ywanie Z-Bufora
	glEnable(GL_COLOR_MATERIAL); //glColor3d ma modyfikowa� w�asno�ci materia�u

	glfwSetKeyCallback(window, key_callback);

}

//Procedura rysuj�ca zawarto�� sceny
void drawScene(GLFWwindow* window, float angle1, float angle2) {
	//************Tutaj umieszczaj kod rysuj�cy obraz******************l

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wykonaj czyszczenie bufora kolor�w


	glm::mat4 V = glm::lookAt( //Wylicz macierz widoku
		glm::vec3(0.0f, 0.0f, observerz),
		glm::vec3(observerx, observery, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 P = glm::perspective(45 * PI / 180, width/height, 1.0f, 150.0f); //Wylicz macierz rzutowania

																	  //Za�aduj macierz rzutowania do OpenGL
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(P));

	//Przejd� w tryb pracy z macierz� Model-Widok
	glMatrixMode(GL_MODELVIEW);

	//Wylicz macierz obrotu o k�t angle wok� osi (0,1,1)
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::rotate(M, angle1, glm::vec3(1, 0, 0));
	M = glm::rotate(M, angle2, glm::vec3(0, 1, 0));
	glLoadMatrixf(glm::value_ptr(V*M)); //Za�aduj wyliczon� macierz do OpenGL


										//Narysuj model
	glEnableClientState(GL_VERTEX_ARRAY); //W��cz u�ywanie tablicy wierzcho�k�w
	glEnableClientState(GL_COLOR_ARRAY); //W��cz u�ywanie tablicy kolor�w

	//glVertexPointer(3, GL_FLOAT, 0, myCubeVertices); //Wska� tablic� wierzcho�k�w
	//glColorPointer(3, GL_FLOAT, 0, myCubeColors); //Wska� tablic� kolor�w

	//glDrawArrays(GL_QUADS, 0, myCubeVertexCount); //Wykonaj rysowanie
	
	for (int i = 0;i < 81;i++) {
		if (i == numer_pola) continue;
		else {
			if (i % 2 == 0) glColor3f(1.0, 0, 0);
			else glColor3f(0, 1.0, 0);
			glRectf(tab[i].x_left, tab[i].y_up, tab[i].x_right, tab[i].y_down);
		}
	}
	
	glColor3f(0, 0, 1.0);
	glRectf(tab[numer_pola].x_left, tab[numer_pola].y_up, tab[numer_pola].x_right, tab[numer_pola].y_down);

	glDisableClientState(GL_VERTEX_ARRAY); //Wy��cz u�ywanie tablicy wierzcho�k�w
	glDisableClientState(GL_COLOR_ARRAY); //Wy��cz u�ywanie tablicy kolor�w

										  //Przerzu� tylny bufor na przedni
	glfwSwapBuffers(window);

}

int main(void)
{
	inicjalizacja(tab);
	for (int i = 0; i < 81; i++) {
		std::cout << tab[i].x_left << " " << tab[i].x_right << " " << tab[i].y_down << " " << tab[i].y_up << std::endl;
	}
	GLFWwindow* window; //Wska�nik na obiekt reprezentuj�cy okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedur� obs�ugi b��d�w

	if (!glfwInit()) { //Zainicjuj bibliotek� GLFW
		fprintf(stderr, "Nie mo�na zainicjowa� GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(width, height, "Pipe", NULL, NULL);  //Utw�rz okno 500x500 o tytule "OpenGL" i kontekst OpenGL. 
	if (!window) //Je�eli okna nie uda�o si� utworzy�, to zamknij program
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje si� aktywny i polecenia OpenGL b�d� dotyczy� w�a�nie jego.
	glfwSwapInterval(1); //Czekaj na 1 powr�t plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotek� GLEW
		fprintf(stderr, "Nie mo�na zainicjowa� GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjuj�ce

	float angle1 = 0;
	float angle2 = 0;
	glfwSetTime(0); //Wyzeruj licznik czasu
	//glutReshapeFunc(Reshape);

					//G��wna p�tla
	while (!glfwWindowShouldClose(window)) //Tak d�ugo jak okno nie powinno zosta� zamkni�te
	{
		angle1 += speedx*glfwGetTime(); //Zwi�ksz k�t o pr�dko�� k�tow� razy czas jaki up�yn�� od poprzedniej klatki
		angle2 += speedy*glfwGetTime();
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window, angle1, angle2); //Wykonaj procedur� rysuj�c�
		glfwPollEvents(); //Wykonaj procedury callback w zalezno�ci od zdarze� jakie zasz�y.
	}

	glfwDestroyWindow(window); //Usu� kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zaj�te przez GLFW
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