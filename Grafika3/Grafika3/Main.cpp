/*
Niniejszy program jest wolnym oprogramowaniem; mo¿esz go
rozprowadzaæ dalej i / lub modyfikowaæ na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundacjê Wolnego
Oprogramowania - wed³ug wersji 2 tej Licencji lub(wed³ug twojego
wyboru) którejœ z póŸniejszych wersji.

Niniejszy program rozpowszechniany jest z nadziej¹, i¿ bêdzie on
u¿yteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyœlnej
gwarancji PRZYDATNOŒCI HANDLOWEJ albo PRZYDATNOŒCI DO OKREŒLONYCH
ZASTOSOWAÑ.W celu uzyskania bli¿szych informacji siêgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnoœci¹ wraz z niniejszym programem otrzyma³eœ te¿ egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeœli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <fstream>
#include "lodepng.h"
#include "shaderprogram.h"

using namespace glm;

float speedx = 0; // [radiany/s]
float speedy = 0;
float observerx = 0.0;
float observery = 0.0;
float observerz = -40.0;
float height = 768;
float width = 1366;
int numer_pola = 40;

				   //Uchwyty na shadery
ShaderProgram *shaderProgram; //WskaŸnik na obiekt reprezentuj¹cy program cieniuj¹cy.

							  //Uchwyty na VAO i bufory wierzcho³ków
GLuint vao;
GLuint bufVertices; //Uchwyt na bufor VBO przechowuj¹cy tablicê wspó³rzêdnych wierzcho³ków
GLuint bufColors;  //Uchwyt na bufor VBO przechowuj¹cy tablicê kolorów
GLuint bufNormals; //Uchwyt na bufor VBO przechowuj¹cy tablickê wektorów normalnych
GLuint bufTexCoords; //Uchwyt na bufor VBO przechowuj¹cy tablickê wektorów normalnych

					 //Kostka
					 /*float* vertices=Models::CubeInternal::vertices;
					 float* colors=Models::CubeInternal::colors;
					 float* normals=Models::CubeInternal::normals;
					 float* texCoords=Models::CubeInternal::texCoords;
					 int vertexCount=Models::CubeInternal::vertexCount;*/

					 //Czajnik
float* vertices;
float* colors;
float* normals;
float* texCoords;
int vertexCount;

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


GLuint readTexture(char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamiêci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
							  //Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamiêci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
									   //Wczytaj obrazek do pamiêci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

//Procedura obs³ugi b³êdów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura obs³ugi klawiatury
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

//Tworzy bufor VBO z tablicy
GLuint makeBuffer(void *data, int vertexCount, int vertexSize) {
	GLuint handle;

	glGenBuffers(1, &handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który bêdzie zawiera³ tablicê danych
	glBindBuffer(GL_ARRAY_BUFFER, handle);  //Uaktywnij wygenerowany uchwyt VBO 
	glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablicê do VBO

	return handle;
}

//Przypisuje bufor VBO do atrybutu 
void assignVBOtoAttribute(ShaderProgram *shaderProgram, char* attributeName, GLuint bufVBO, int vertexSize) {
	GLuint location = shaderProgram->getAttribLocation(attributeName); //Pobierz numery slotów dla atrybutu
	glBindBuffer(GL_ARRAY_BUFFER, bufVBO);  //Uaktywnij uchwyt VBO 
	glEnableVertexAttribArray(location); //W³¹cz u¿ywanie atrybutu o numerze slotu zapisanym w zmiennej location
	glVertexAttribPointer(location, vertexSize, GL_FLOAT, GL_FALSE, 0, NULL); //Dane do slotu location maj¹ byæ brane z aktywnego VBO
}


//Procedura inicjuj¹ca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który nale¿y wykonaæ raz, na pocz¹tku programu************
	glClearColor(1, 1, 1, 1); //Czyœæ ekran na czarno	
	
	//glEnable(GL_LIGHT0); //W³¹cz domyslne œwiat³o NIE WIEM, CZY POWINO BYC

	glEnable(GL_DEPTH_TEST); //W³¹cz u¿ywanie Z-Bufora
	glEnable(GL_COLOR_MATERIAL); //glColor3d ma modyfikowaæ w³asnoœci materia³u
	glfwSetKeyCallback(window, key_callback); //Zarejestruj procedurê obs³ugi klawiatury


	shaderProgram = new ShaderProgram("vshader.txt", NULL, "fshader.txt"); //Wczytaj program cieniuj¹cy 


																		   //*****Przygotowanie do rysowania pojedynczego obiektu*******
																		   //Zbuduj VBO z danymi obiektu do narysowania
	bufVertices = makeBuffer(vertices, vertexCount, sizeof(float) * 4); //VBO ze wspó³rzêdnymi wierzcho³ków
	bufColors = makeBuffer(colors, vertexCount, sizeof(float) * 4);//VBO z kolorami wierzcho³ków
	bufNormals = makeBuffer(normals, vertexCount, sizeof(float) * 4);//VBO z wektorami normalnymi wierzcho³ków
	bufTexCoords = makeBuffer(texCoords, vertexCount, sizeof(float) * 2);//VBO ze wspó³rzêdnymi teksturowania

																		 //Zbuduj VAO wi¹¿¹cy atrybuty z konkretnymi VBO
	glGenVertexArrays(1, &vao); //Wygeneruj uchwyt na VAO i zapisz go do zmiennej globalnej

	glBindVertexArray(vao); //Uaktywnij nowo utworzony VAO

	assignVBOtoAttribute(shaderProgram, "vertex", bufVertices, 4); //"vertex" odnosi siê do deklaracji "in vec4 vertex;" w vertex shaderze
	assignVBOtoAttribute(shaderProgram, "color", bufColors, 4); //"color" odnosi siê do deklaracji "in vec4 color;" w vertex shaderze
	assignVBOtoAttribute(shaderProgram, "normal", bufNormals, 4); //"normal" odnosi siê do deklaracji "in vec4 normal;" w vertex shaderze
	assignVBOtoAttribute(shaderProgram, "texCoord0", bufTexCoords, 2); //"texCoord0" odnosi siê do deklaracji "in vec2 texCoord0;" w vertex shaderze

	glBindVertexArray(0); //Dezaktywuj VAO
						  //******Koniec przygotowania obiektu************

	tex0 = readTexture("metal.png");
}

//Zwolnienie zasobów zajêtych przez program
void freeOpenGLProgram() {
	delete shaderProgram; //Usuniêcie programu cieniuj¹cego

	glDeleteVertexArrays(1, &vao); //Usuniêcie vao
	glDeleteBuffers(1, &bufVertices); //Usuniêcie VBO z wierzcho³kami
	glDeleteBuffers(1, &bufColors); //Usuniêcie VBO z kolorami
	glDeleteBuffers(1, &bufNormals); //Usuniêcie VBO z wektorami normalnymi
	glDeleteBuffers(1, &bufTexCoords); //Usuniêcie VBO ze wspó³rzêdnymi teksturowania


}

void drawObject(GLuint vao, ShaderProgram *shaderProgram, mat4 mP, mat4 mV, mat4 mM) {
	//W³¹czenie programu cieniuj¹cego, który ma zostaæ u¿yty do rysowania
	//W tym programie wystarczy³oby wywo³aæ to raz, w setupShaders, ale chodzi o pokazanie, 
	//¿e mozna zmieniaæ program cieniuj¹cy podczas rysowania jednej sceny
	shaderProgram->use();

	//Przeka¿ do shadera macierze P,V i M.
	//W linijkach poni¿ej, polecenie:
	//  shaderProgram->getUniformLocation("P") 
	//pobiera numer slotu odpowiadaj¹cego zmiennej jednorodnej o podanej nazwie
	//UWAGA! "P" w powy¿szym poleceniu odpowiada deklaracji "uniform mat4 P;" w vertex shaderze, 
	//a mP w glm::value_ptr(mP) odpowiada argumentowi  "mat4 mP;" TYM pliku.
	//Ca³a poni¿sza linijka przekazuje do zmiennej jednorodnej P w vertex shaderze dane z argumentu mP niniejszej funkcji
	//Pozosta³e polecenia dzia³aj¹ podobnie.
	glUniformMatrix4fv(shaderProgram->getUniformLocation("P"), 1, false, glm::value_ptr(mP));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("V"), 1, false, glm::value_ptr(mV));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(mM));

	glUniform4f(shaderProgram->getUniformLocation("lightPos0"), 0, 0, 5, 1); //Przekazanie wspó³rzêdnych Ÿród³a œwiat³a do zmiennej jednorodnej lightPos0
																			  //Powi¹zanie zmiennych jednorodnych textureMap0 i textureMap1 z odpowiednimi jednostkami teksturuj¹cymi
	glUniform1i(shaderProgram->getUniformLocation("textureMap0"), 0);
	glUniform1i(shaderProgram->getUniformLocation("textureMap1"), 1);

	//Przypisanie tekstur do jednostek teksturuj¹cych
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex0);

	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao);

	//Narysowanie obiektu
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);

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

	//Posprz¹tanie po sobie (niekonieczne w sumie je¿eli korzystamy z VAO dla ka¿dego rysowanego obiektu)
	glBindVertexArray(0);
}

//Procedura rysuj¹ca zawartoœæ sceny
void drawScene(GLFWwindow* window, float angle1, float angle2) {
	//************Tutaj umieszczaj kod rysuj¹cy obraz******************l
	//glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wykonaj czyszczenie bufora kolorów

	glm::mat4 P = glm::perspective(60 * 3.14f / 180, width / height, 1.0f, 150.0f); //Wylicz macierz rzutowania

	glm::mat4 V = glm::lookAt( //Wylicz macierz widoku
		glm::vec3(0.0f, 0.0f, observerz),
		glm::vec3(observerx, observery, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(P));

	//PrzejdŸ w tryb pracy z macierz¹ Model-Widok
	glMatrixMode(GL_MODELVIEW);

	//Wylicz macierz modelu rysowanego obiektu
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::rotate(M, angle1, glm::vec3(1, 0, 0));
	M = glm::rotate(M, angle2, glm::vec3(0, 1, 0));
	glLoadMatrixf(glm::value_ptr(V*M)); //Za³aduj wyliczon¹ macierz do OpenGL

	
	//glBindVertexArray(vao);

	//glEnableClientState(GL_VERTEX_ARRAY); //W³¹cz u¿ywanie tablicy wierzcho³ków
	//glEnableClientState(GL_COLOR_ARRAY); //W³¹cz u¿ywanie tablicy kolorów

	//glDisableClientState(GL_VERTEX_ARRAY); //Wy³¹cz u¿ywanie tablicy wierzcho³ków
	//glDisableClientState(GL_COLOR_ARRAY); //Wy³¹cz u¿ywanie tablicy kolorów
	//glBindVertexArray(0);

	drawObject(vao, shaderProgram, P, V, M);

	//Przerzuæ tylny bufor na przedni
	glfwSwapBuffers(window);

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
		fprintf(stderr, "Nie mo¿na utworzyæ okna.\n");
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

	float angle1 = 0; //K¹t obrotu obiektu
	float angle2 = 0; //K¹t obrotu obiektu

	glfwSetTime(0); //Wyzeruj licznik czasu

					//G³ówna pêtla
	while (!glfwWindowShouldClose(window)) //Tak d³ugo jak okno nie powinno zostaæ zamkniête
	{
		angle1 += speedx*glfwGetTime(); //Zwiêksz k¹t o prêdkoœæ k¹tow¹ razy czas jaki up³yn¹³ od poprzedniej klatki
		angle2 += speedy*glfwGetTime(); //Zwiêksz k¹t o prêdkoœæ k¹tow¹ razy czas jaki up³yn¹³ od poprzedniej klatki
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window, angle1, angle2); //Wykonaj procedurê rysuj¹c¹
		glfwPollEvents(); //Wykonaj procedury callback w zaleznoœci od zdarzeñ jakie zasz³y.
	}

	freeOpenGLProgram();

	glfwDestroyWindow(window); //Usuñ kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajête przez GLFW
	exit(EXIT_SUCCESS);
}