#include <stdio.h>
#include <vector>
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "tga.h"
#include "shaderprogram.h"


int numer_pola = 24;
int kamera = 0;
int status = 0;

struct Pola {
	int pozycja;            // u³o¿enie elementu 1-4 (tak jak jest u³o¿ony)
	int model;              // który element ma tam staæ 1-5
	float wsp_x;            // wspó³rzêdne x
	float wsp_z;            // wspó³rzêdne z
	int obrot;
};
Pola rura[49];


//Macierze
glm::mat4  matM;
glm::mat4  matV;
glm::mat4  matP;
glm::mat4  model[49];


bool* keyStates = new bool[10];

//Ustawienia okna i rzutowania
int winPosX = 0;
int winPosY = 0;
int Height = 650;
int Width = 1050;
float cameraAngle = 45.0f;

//Zmienne do animacji
///int lastTime = 0;
int camera = 1;
float cam1 = 0.0f;

//Uchwyty na shadery
ShaderProgram *shaderProgram; //WskaŸnik na obiekt reprezentuj¹cy program cieniuj¹cy.

#define N 6

GLuint vao[N];
GLuint bufVertices[N]; //Uchwyt na bufor VBO przechowuj¹cy tablicê wspó³rzêdnych wierzcho³ków
GLuint bufColors[N];  //Uchwyt na bufor VBO przechowuj¹cy tablicê kolorów
GLuint bufNormals[N]; //Uchwyt na bufor VBO przechowuj¹cy tablickê wektorów normalnych
GLuint tex0[N];
GLuint bufTexCoords[N];
GLuint texZaznaczony;
GLuint texKoniec;



std::vector<glm::vec4> vertys[N];
std::vector<glm::vec2> uvs[N];
std::vector<glm::vec4> nrmals[N];

float *verts;
float *colors;
float *normals;
int vertexCount[N];


void ok() //poprawne ustawienie rur
{

	for (int i = 0; i < 49; i++)
	{
		int pom = rura[i].pozycja;
		while (pom != 0)
		{
			model[i] = glm::rotate(model[i], -90.0f, glm::vec3(0, 1, 0));
			pom++;
			if (pom > 3) pom = 0;
			rura[i].pozycja = pom;
		}
	}


}

void los()
{
	// dla elementów 18, 29, 38 nie losujemy, bo pozycja zawsze równa 0
	// dla elementów i == 4 || i == 5 || i == 9 || i == 17 || i == 23 || i == 27 || i == 31 || i == 35 || i == 36 || i == 46 || i == 47 losujemy tylko miêdzy 0 i 1
	// dla reszty losujemy 0-3
	int obrot = 0;
	for (int i = 0; i < 49; i++)
	{
		if (i == 18 || i == 29 || i == 38) i++;

		if (i == 4 || i == 5 || i == 9 || i == 17 || i == 23 || i == 27 || i == 31 || i == 35 || i == 36 || i == 46 || i == 47)
		{
			obrot = rand() % 2;
			while (obrot != 0)
			{
				model[i] = glm::rotate(model[i], -90.0f, glm::vec3(0, 1, 0));
				rura[i].pozycja++;
				if (rura[i].pozycja == 2) rura[i].pozycja = 0;
				obrot--;
			}
		}
		else
		{
			obrot = rand() % 4;
			while (obrot != 0)
			{
				model[i] = glm::rotate(model[i], -90.0f, glm::vec3(0, 1, 0));
				rura[i].pozycja++;
				if (rura[i].pozycja == 4) rura[i].pozycja = 0;
				obrot--;
			}
		}
	}

}


void czyJestOk()
{
	int zle = 0;
	for (int i = 0; i < 49; i++)
	{
		if (rura[i].pozycja != 0) zle++;
	}
	if (zle != 0) status = 0;
	else status = 1;
}


void drawObject() {

	shaderProgram->use();
	czyJestOk();

	glUniformMatrix4fv(shaderProgram->getUniformLocation("P"), 1, false, glm::value_ptr(matP));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("V"), 1, false, glm::value_ptr(matV));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(matM));
	glUniform1i(shaderProgram->getUniformLocation("textureMap0"), 0);


	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(glm::scale(glm::mat4(1), glm::vec3(40.5))));
	glBindTexture(GL_TEXTURE_2D, tex0[0]);
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[0]);
	//Narysowanie obiektu
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[0]);

	if (status == 0)
	{
		
		int k = 0;

		for (int j = 0; j < 7; j++)
		{
			for (int i = 0; i < 7; i++)
			{
				
				if (rura[k].obrot == 1) {
					model[k] = glm::rotate(model[k], -90.0f, glm::vec3(0, 1, 0));
					rura[k].obrot = 0;
					if (k == 18 || k == 29 || k == 38)
					{
						rura[k].pozycja = 0;
					}
					else if (k == 4 || k == 5 || k == 9 || k == 17 || k == 23 || k == 27 || k == 31 || k == 35 || k == 36 || k == 46 || k == 47)
					{
						rura[k].pozycja += 1;
						if (rura[k].pozycja > 1) rura[k].pozycja = 0;
					}
					else
					{
						rura[k].pozycja += 1;
						if (rura[k].pozycja > 3) rura[k].pozycja = 0;
					}

				}

				glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(glm::scale(model[k], glm::vec3(1))));

				glBindVertexArray(vao[rura[k].model]);
				if (k == numer_pola) { glBindTexture(GL_TEXTURE_2D, texZaznaczony); }
				else    glBindTexture(GL_TEXTURE_2D, tex0[rura[k].model]);
				glDrawArrays(GL_TRIANGLES, 0, vertexCount[rura[k].model]);

				k++;
			}
		}
	}
	else
	{
		int k = 0;
		for (int j = 0; j < 7; j++)
		{
			for (int i = 0; i < 7; i++)
			{
				glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(glm::scale(model[k], glm::vec3(1))));

				glBindVertexArray(vao[rura[k].model]);
				//		if (k == numer_pola) { glBindTexture(GL_TEXTURE_2D, textura); }
				glBindTexture(GL_TEXTURE_2D, texKoniec);
				glDrawArrays(GL_TRIANGLES, 0, vertexCount[rura[k].model]);

				k++;
			}
		}

	}

}

//Procedura rysuj¹ca
void drawScene()
{

	//Wyczyœæ bufor kolorów i bufor g³êbokoœci
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Wylicz macierz rzutowania
	matP = glm::perspective(cameraAngle, (float)Width / (float)Height, 1.0f, 100.0f);

	//Wylicz macierz widoku
	switch (kamera)
	{
	case 0:
		matV = glm::lookAt(
			glm::vec3(0.0f, 30.0f + (2 / 3 * cam1), 45.0f + cam1),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		break;

	case 1:
		matV = glm::lookAt(
			glm::vec3(0.0f, 60.0f + cam1, 1.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	}

	drawObject();
	glutSwapBuffers();

}

GLuint makeBuffer(std::vector<glm::vec4> data)
{
	GLuint handle;

	glGenBuffers(1, &handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który bêdzie zawiera³ tablicê danych
	glBindBuffer(GL_ARRAY_BUFFER, handle);  //Uaktywnij wygenerowany uchwyt VBO
											//glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablicê do VBO
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec4), &data[0], GL_STATIC_DRAW);
	return handle;
}

GLuint makeBuffer(std::vector<glm::vec2> data) {
	GLuint handle;

	glGenBuffers(1, &handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który bêdzie zawiera³ tablicê danych
	glBindBuffer(GL_ARRAY_BUFFER, handle);  //Uaktywnij wygenerowany uchwyt VBO
											//glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablicê do VBO
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec2), &data[0], GL_STATIC_DRAW);
	return handle;
}

GLuint makeBuffer(void *data, int vertexCount, int vertexSize) {
	GLuint handle;

	glGenBuffers(1, &handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który bêdzie zawiera³ tablicê danych
	glBindBuffer(GL_ARRAY_BUFFER, handle);  //Uaktywnij wygenerowany uchwyt VBO
	glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablicê do VBO

	return handle;
}

//Procedura tworz¹ca bufory VBO zawieraj¹ce dane z tablic opisuj¹cych rysowany obiekt.
void setupVBO() {
	for (int i = 0; i<N; i++) {
		bufVertices[i] = makeBuffer(vertys[i]);
		bufColors[i] = makeBuffer(vertys[i]);
		bufNormals[i] = makeBuffer(nrmals[i]);
		bufTexCoords[i] = makeBuffer(uvs[i]);

	}
}

void assignVBOtoAttribute(char* attributeName, GLuint bufVBO, int variableSize) {
	GLuint location = shaderProgram->getAttribLocation(attributeName); //Pobierz numery slotów dla atrybutu
	glBindBuffer(GL_ARRAY_BUFFER, bufVBO);  //Uaktywnij uchwyt VBO
	glEnableVertexAttribArray(location); //W³¹cz u¿ywanie atrybutu o numerze slotu zapisanym w zmiennej location
	glVertexAttribPointer(location, variableSize, GL_FLOAT, GL_FALSE, 0, NULL); //Dane do slotu location maj¹ byæ brane z aktywnego VBO
}

void assignVBOtoAttribute(ShaderProgram *shader, char* attributeName, GLuint bufVBO, int variableSize) {
	GLuint location = shader->getAttribLocation(attributeName); //Pobierz numery slotów dla atrybutu
	glBindBuffer(GL_ARRAY_BUFFER, bufVBO);  //Uaktywnij uchwyt VBO
	glEnableVertexAttribArray(location); //W³¹cz u¿ywanie atrybutu o numerze slotu zapisanym w zmiennej location
	glVertexAttribPointer(location, variableSize, GL_FLOAT, GL_FALSE, 0, NULL); //Dane do slotu location maj¹ byæ brane z aktywnego VBO (null oznacza aktywny VBO)
}

//Procedura tworz¹ca VAO - "obiekt" OpenGL wi¹¿¹cy numery slotów atrybutów z buforami VBO
void setupVAO() {
	for (int i = 0; i<N; i++)
	{
		//Wygeneruj uchwyt na VAO i zapisz go do zmiennej globalnej
		glGenVertexArrays(1, &vao[i]);

		//Uaktywnij nowo utworzony VAO
		glBindVertexArray(vao[i]);

		assignVBOtoAttribute(shaderProgram, "vertex", bufVertices[i], 4); //"vertex" odnosi siê do deklaracji "in vec4 vertex;" w vertex shaderze
		assignVBOtoAttribute(shaderProgram, "color", bufColors[i], 4); //"color" odnosi siê do deklaracji "in vec4 color;" w vertex shaderze
		assignVBOtoAttribute(shaderProgram, "normal", bufNormals[i], 4); //"normal" odnosi siê do deklaracji "in vec4 normal;" w vertex shaderze
		assignVBOtoAttribute(shaderProgram, "texCoord", bufTexCoords[i], 2);


		//assignVBOtoAttribute(shaderProgram3,"vertex",bufVertices[i],4); //"vertex" odnosi siê do deklaracji "in vec4 vertex;" w vertex shaderze
		//assignVBOtoAttribute(shaderProgram3,"color",bufVertices[i],4); //"color" odnosi siê do deklaracji "in vec4 color;" w vertex shaderze
	}
	glBindVertexArray(0);
}

//Procedura uruchamiana okresowo. Robi animacjê.
void nextAction(void) {
	int actTime = glutGet(GLUT_ELAPSED_TIME);
	glutPostRedisplay();
}

bool loadOBJ(const char * path, std::vector<glm::vec4> & out_vertices, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec4> & out_normals)
{
	printf("Loading %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec4> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec4> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Can't open the file...\n");
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

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
				//  return false;
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


//Procedura wywo³ywana przy zmianie rozmiaru okna
void changeWindowSize(int w, int h) {
	//Ustawienie wymiarow przestrzeni okna
	glViewport(0, 0, w, h);
	//Zapamiêtanie nowych wymiarów okna dla poprawnego wyliczania macierzy rzutowania
	Width = w;
	Height = h;
}

//Procedura inicjuj¹ca biblotekê glut
void initGLUT(int *argc, char** argv) {
	glutInit(argc, argv); //Zainicjuj bibliotekê GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); //Alokuj bufory kolorów (podwójne buforowanie) i bufor kolorów

	glutInitWindowPosition(winPosX, winPosY); //Wska¿ pocz¹tkow¹ pozycjê okna
	glutInitWindowSize(Width, Height); //Wska¿ pocz¹tkowy rozmiar okna
	glutCreateWindow("Pipe"); //Utwórz okno i nadaj mu tytu³

	glutReshapeFunc(changeWindowSize); //Zarejestruj procedurê changeSize jako procedurê obs³uguj¹ca zmianê rozmiaru okna
	glutDisplayFunc(drawScene); //Zarejestruj procedurê displayFrame jako procedurê obs³uguj¹ca odœwierzanie okna
	glutIdleFunc(nextAction); //Zarejestruj procedurê nextFrame jako procedurê wywo³ywan¹ najczêœciêj jak siê da (animacja)
}


//Procedura inicjuj¹ca bibliotekê glew
void initGLEW() {
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "%s\n", glewGetErrorString(err));
		exit(1);
	}

}



//Wczytuje vertex shader i fragment shader i ³¹czy je w program cieniuj¹cy
void setupShaders() {
	shaderProgram = new ShaderProgram("vshader.txt", NULL, "fshader.txt");
}

//procedura inicjuj¹ca ró¿ne sprawy zwi¹zane z rysowaniem w OpenGL
void initOpenGL() {
	setupShaders();
	setupVBO();
	setupVAO();
	glEnable(GL_DEPTH_TEST);
}

//Zwolnij pamiêæ karty graficznej z shaderów i programu cieniuj¹cego
void cleanShaders() {
	delete shaderProgram;
}

void freeVBO() {
	for (int i = 0; i<N; i++) {
		glDeleteBuffers(1, &bufVertices[i]);
		glDeleteBuffers(1, &bufColors[i]);
		glDeleteBuffers(1, &bufNormals[i]);
	}
}

void freeVAO() {
	for (int i = 0; i<N; i++) {
		glDeleteVertexArrays(1, &vao[i]);
	}
}

GLuint readTexture(char* filename) {
	GLuint tex;
	TGAImg img;
	glActiveTexture(GL_TEXTURE0);
	if (img.Load(filename) == IMG_OK) {
		glGenTextures(1, &tex); //Zainicjuj uchwyt tex
		glBindTexture(GL_TEXTURE_2D, tex); //Przetwarzaj uchwyt tex
		if (img.GetBPP() == 24) //Obrazek 24bit
			glTexImage2D(GL_TEXTURE_2D, 0, 3, img.GetWidth(), img.GetHeight(), 0,
				GL_RGB, GL_UNSIGNED_BYTE, img.GetImg());
		else if (img.GetBPP() == 32) //Obrazek 32bit
			glTexImage2D(GL_TEXTURE_2D, 0, 4, img.GetWidth(), img.GetHeight(), 0,
				GL_RGBA, GL_UNSIGNED_BYTE, img.GetImg());
		else {
			printf("Nieobs³ugiwany format obrazka w pliku: %s \n", filename);
		}
	}
	else {
		printf("B³¹d przy wczytywaniu pliku: %s\n", filename);
	}
	//MIPMAPPING
	glGenerateMipmap(GL_TEXTURE_2D);//wygenerowanie mipmapy
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);//uzywa dla najblizszego sasiedztwa najnizszy poziom mipmap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return tex;
}




void keyFunc() {


	if (keyStates['r'] == true) {
		los();
		status = 0;
	}
	if (keyStates[13] == true) {
		if (status == 0)
		{
			rura[numer_pola].obrot = 1;
		}

	}
	if (keyStates[32] == true) {
		if (kamera == 1) kamera = 0;
		else kamera = 1;
	}



	if (keyStates['='] == true) {
		if (cam1>-40)cam1 -= 5;
		else cam1 = -40;

	}
	if (keyStates['-'] == true) {
		if (cam1<30)cam1 += 5;
		else cam1 = 30;
	}
}

void keyPressed(unsigned char key, int x, int y) {
	keyStates[key] = true;
	keyFunc();
}


void keyUpFunc(unsigned char key, int x, int y) {
	keyStates[key] = false;
}

void SpecialInput(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		if (numer_pola > 6) numer_pola -= 7;
		else numer_pola = numer_pola + 49 - 7;
		break;
	case GLUT_KEY_DOWN:
		if (numer_pola < 42) numer_pola += 7;
		else numer_pola = numer_pola + 7 - 49;
		break;
	case GLUT_KEY_RIGHT:
		if (numer_pola % 7 != 6) numer_pola += 1;
		else numer_pola = numer_pola - 6;
		break;
	case GLUT_KEY_LEFT:
		if (numer_pola % 7 != 0) numer_pola -= 1;
		else numer_pola = numer_pola + 6;
		break;
	}
}

void loadModels()
{
	loadOBJ("plane.obj", vertys[0], uvs[0], nrmals[0]);
	vertexCount[0] = vertys[0].size();
	loadOBJ("1_out_pipe2.obj", vertys[1], uvs[1], nrmals[1]);
	vertexCount[1] = vertys[1].size();
	loadOBJ("2_out_broken_pipe.obj", vertys[2], uvs[2], nrmals[2]);
	vertexCount[2] = vertys[2].size();
	loadOBJ("2_out_connected_pipe.obj", vertys[3], uvs[3], nrmals[3]);
	vertexCount[3] = vertys[3].size();
	loadOBJ("3_out_connected_pipe.obj", vertys[4], uvs[4], nrmals[4]);
	vertexCount[4] = vertys[4].size();
	loadOBJ("4_out_pipe.obj", vertys[5], uvs[5], nrmals[5]);
	vertexCount[5] = vertys[5].size();
}

void inicjalizacja()
{
	for (int i = 0; i < 49; i++)
	{

		if (i == 18 || i == 29 || i == 38)
		{
			rura[i].model = 5;
		}
		else if (i == 0 || i == 24 || i == 40)
		{
			rura[i].model = 1;
		}
		else if (i == 8 || i == 16 || i == 19 || i == 21 || i == 28 || i == 33 || i == 45)
		{
			rura[i].model = 4;
		}
		else if (i == 4 || i == 5 || i == 9 || i == 17 || i == 23 || i == 27 || i == 31 || i == 35 || i == 36 || i == 46 || i == 47)
		{
			rura[i].model = 3;
		}
		else rura[i].model = 2;

		if (i == 7 || i == 10 || i == 25 || i == 40 || i == 42 || i == 44)
		{
			rura[i].pozycja = 3;
		}
		else if (i == 0 || i == 1 || i == 3 || i == 8 || i == 12 || i == 14 || i == 24 || i == 32 || i == 37)
		{
			rura[i].pozycja = 2;
		}
		else if (i == 2 || i == 4 || i == 5 || i == 6 || i == 11 || i == 17 || i == 19 || i == 20 || i == 22 || i == 26 || i == 33 || i == 41 || i == 45 || i == 46 || i == 47)
		{
			rura[i].pozycja = 1;
		}
		else rura[i].pozycja = 0;

	}
	for (int i = 0; i < N; i++) {
		rura[i].obrot = 0;
	}

	float x = -17.25;
	float z = -17.25;
	int k = 0;

	for (int j = 0; j < 7; j++)
	{
		for (int i = 0; i < 7; i++)
		{
			rura[k].wsp_x = x;
			rura[k].wsp_z = z;
			model[k] = glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(x, 1, z, 1));
			k++;
			x += 5.75;
		}

		z += 5.75;
		x = -17.25;
	}

}

int main(int argc, char** argv) {

	inicjalizacja();
	los();
	//  ok();
	//  czyJestOk();
	loadModels();


	initGLUT(&argc, argv);
	initGLEW();
	initOpenGL();



	glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyUpFunc);
	glutSpecialFunc(SpecialInput);



	tex0[0] = readTexture("6.tga");
	for (int i = 1; i < N; i++)
	{
		tex0[i] = readTexture("11.tga");
	}
	texZaznaczony = readTexture("dark11.tga");
	texKoniec = readTexture("green.tga");

	glutMainLoop();
	freeVAO();
	freeVBO();
	cleanShaders();
	return 0;
}