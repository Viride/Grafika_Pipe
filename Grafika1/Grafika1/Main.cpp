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
#include <fstream>
#include "colors.h"
#include "materials.h"
#include "shaderprogram.h"
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "myCube.h"
// wskaŸnik na funkcjê glWindowPos2i

float speedx = 0; // [radiany/s]
float speedy = 0;
float observerx = 0.0;
float observery = 0.0;
float observerz = -40.0;
float height = 768;
float width = 1366;
int numer_pola=40;

//Uchwyty na shadery
ShaderProgram *shaderProgram; //WskaŸnik na obiekt reprezentuj¹cy program cieniuj¹cy.

							  //Uchwyty na VAO i bufory wierzcho³ków
GLuint vao;
GLuint bufVertices; //Uchwyt na bufor VBO przechowuj¹cy tablicê wspó³rzêdnych wierzcho³ków
GLuint bufColors;  //Uchwyt na bufor VBO przechowuj¹cy tablicê kolorów
GLuint bufNormals; //Uchwyt na bufor VBO przechowuj¹cy tablickê wektorów normalnych
GLuint bufTexCoords; //Uchwyt na bufor VBO przechowuj¹cy tablickê wektorów normalnych
GLuint tex0;

struct Pola {
	int number;				//numer elementu 0-80
	float x_left, x_right;	// wspó³rzêdne do kwadratu
	float y_up, y_down;		// wspó³rzêdne do kwadratu
	int pozycja;			// u³o¿enie elementu 1-4 (tak jak jest u³o¿ony)
	int model;				// który element ma tam staæ
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


GLuint makeBuffer(std::vector<glm::vec4> data) {
	GLuint handle;

	glGenBuffers(1, &handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który bêdzie zawiera³ tablicê danych
	glBindBuffer(GL_ARRAY_BUFFER, handle);  //Uaktywnij wygenerowany uchwyt VBO 
											//glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablicê do VBO
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec4), &data[0], GL_STATIC_DRAW);
	return handle;
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
void drawScene(GLFWwindow* window, float angle1, float angle2) {
	//************Tutaj umieszczaj kod rysuj¹cy obraz******************l

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wykonaj czyszczenie bufora kolorów


	glm::mat4 V = glm::lookAt( //Wylicz macierz widoku
		glm::vec3(0.0f, 0.0f, observerz),
		glm::vec3(observerx, observery, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 P = glm::perspective(60 * PI / 180, width/height, 1.0f, 150.0f); //Wylicz macierz rzutowania

																	  //Za³aduj macierz rzutowania do OpenGL
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(P));

	//PrzejdŸ w tryb pracy z macierz¹ Model-Widok
	glMatrixMode(GL_MODELVIEW);

	//Wylicz macierz obrotu o k¹t angle wokó³ osi (0,1,1)
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::rotate(M, angle1, glm::vec3(1, 0, 0));
	M = glm::rotate(M, angle2, glm::vec3(0, 1, 0));
	glLoadMatrixf(glm::value_ptr(V*M)); //Za³aduj wyliczon¹ macierz do OpenGL


										//Narysuj model
	glEnableClientState(GL_VERTEX_ARRAY); //W³¹cz u¿ywanie tablicy wierzcho³ków
	glEnableClientState(GL_COLOR_ARRAY); //W³¹cz u¿ywanie tablicy kolorów

	//glVertexPointer(3, GL_FLOAT, 0, myCubeVertices); //Wska¿ tablicê wierzcho³ków
	//glColorPointer(3, GL_FLOAT, 0, myCubeColors); //Wska¿ tablicê kolorów

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

	glDisableClientState(GL_VERTEX_ARRAY); //Wy³¹cz u¿ywanie tablicy wierzcho³ków
	glDisableClientState(GL_COLOR_ARRAY); //Wy³¹cz u¿ywanie tablicy kolorów

										  //Przerzuæ tylny bufor na przedni
	glfwSwapBuffers(window);

}

void freeOpenGLProgram() {
	delete shaderProgram; //Usuniêcie programu cieniuj¹cego

	glDeleteVertexArrays(1, &vao); //Usuniêcie vao
	glDeleteBuffers(1, &bufVertices); //Usuniêcie VBO z wierzcho³kami
	glDeleteBuffers(1, &bufColors); //Usuniêcie VBO z kolorami
	glDeleteBuffers(1, &bufNormals); //Usuniêcie VBO z wektorami normalnymi
	glDeleteBuffers(1, &bufTexCoords); //Usuniêcie VBO ze wspó³rzêdnymi teksturowania
}

bool loadOBJ(const char * path, std::vector<glm::vec4> & out_vertices, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec4> & out_normals) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec4> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec4> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ?\n");
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec4 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			vertex.w = 1.0f;
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec4 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normal.w = 0.0f;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				//	return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec4 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec4 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}

int main(void)
{
	inicjalizacja(tab);
	for (int i = 0; i < 81; i++) {
		std::cout << tab[i].x_left << " " << tab[i].x_right << " " << tab[i].y_down << " " << tab[i].y_up << std::endl;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

	float angle1 = 0;
	float angle2 = 0;
	glfwSetTime(0); //Wyzeruj licznik czasu
	//glutReshapeFunc(Reshape);

					//G³ówna pêtla
	while (!glfwWindowShouldClose(window)) //Tak d³ugo jak okno nie powinno zostaæ zamkniête
	{
		angle1 += speedx*glfwGetTime(); //Zwiêksz k¹t o prêdkoœæ k¹tow¹ razy czas jaki up³yn¹³ od poprzedniej klatki
		angle2 += speedy*glfwGetTime();
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window, angle1, angle2); //Wykonaj procedurê rysuj¹c¹
		glfwPollEvents(); //Wykonaj procedury callback w zaleznoœci od zdarzeñ jakie zasz³y.
	}
	
	freeOpenGLProgram();

	glfwDestroyWindow(window); //Usuñ kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajête przez GLFW
	exit(EXIT_SUCCESS);
}