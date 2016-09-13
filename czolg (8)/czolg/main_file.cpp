#include "GL/glew.h"
#include "GL/freeglut.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <stdio.h>
#include "tga.h"
#include "shaderprogram.h"
#include <vector>
#include <fstream>
#include <string>
#include <time.h>
#include <math.h>

int numer_pola = 24;


struct Pola {
	//int numer;				//numer rury
	int pozycja;			// u³o¿enie elementu 1-4 (tak jak jest u³o¿ony)
	int model;				// który element ma tam staæ 1-5
	float wsp_x;			// wspó³rzêdne x
	float wsp_z;			// wspó³rzêdne z
	int obrot;
};
Pola rura[49];


//Macierze
glm::mat4  matP;//rzutowania
glm::mat4  matV;//widoku
glm::mat4  matM;//modelu
glm::mat4  model[49];
glm::mat4  modele;
glm::mat4  matTree;
glm::mat4  matGrass;
glm::mat4  matCastle1;
glm::mat4  matCastle2;
glm::mat4  matCastle3;
glm::mat4  matSky;
glm::vec4 wektor1;
glm::vec4 wektor2;
glm::vec4 wektor3;
glm::vec4 wektor4;

bool* keyStates = new bool[256];
int cameranumber;

float xcam=0.0f;
float ycam=5.0f;
float zcam=9.0f;

float xlook=0.0f;
float ylook=0.0f;
float zlook=0.0f;
float anglecam = 0.0f;
float radius = 9.0f;

GLfloat modelviewMatrix[16];


//Ustawienia okna i rzutowania
int windowPositionX=0;
int windowPositionY=0;
int Width=1050;
int Height=650;
float cameraAngle=45.0f;

//Zmienne do animacji
float speed=120; //120 stopni/s
int lastTime=0;
float angle=0;
float move=0.05;
int speedtank=0;
int camera=1;

//Uchwyty na shadery
ShaderProgram *shaderProgram; //WskaŸnik na obiekt reprezentuj¹cy program cieniuj¹cy.

#define LICZBAELEMENTOW 6

GLuint vao[LICZBAELEMENTOW];
GLuint bufVertices[LICZBAELEMENTOW]; //Uchwyt na bufor VBO przechowuj¹cy tablicê wspó³rzêdnych wierzcho³ków
GLuint bufColors[LICZBAELEMENTOW];  //Uchwyt na bufor VBO przechowuj¹cy tablicê kolorów
GLuint bufNormals[LICZBAELEMENTOW]; //Uchwyt na bufor VBO przechowuj¹cy tablickê wektorów normalnych
GLuint tex0[LICZBAELEMENTOW];
GLuint bufTexCoords[LICZBAELEMENTOW];
GLuint textura;

//"Model" który rysujemy. Dane wskazywane przez poni¿sze wskaŸniki i o zadanej liczbie wierzcho³ków s¹ póŸniej wysowane przez program.
//W programie s¹ dwa modele, z których jeden mo¿na wybraæ komentuj¹c/odkomentowuj¹c jeden z poni¿szych fragmentów.

//Kostka
/*float *vertices=cubeVertices;
float *colors=cubeColors;
float *normals=cubeNormals;
int vertexCount=cubeVertexCount;*/

std::vector<glm::vec4> vertys[LICZBAELEMENTOW];
std::vector<glm::vec2> uvs[LICZBAELEMENTOW];
std::vector<glm::vec4> nrmals[LICZBAELEMENTOW];

float *verts;
float *colors;
float *normals;
int vertexCount[LICZBAELEMENTOW];


void ok() //poprawne ustawienie rur
{

	for (int i = 0; i < 49; i++)
	{
		int pom = rura[i].pozycja;
		while (pom != 0)
		{
			// tutaj te obroty
			pom++;
			if (pom > 4) pom = 0;
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
				// TUTAJ OBROT O 90 RURY
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
				// TUTAJ OBROT O 90 RURY
				rura[i].pozycja++;
				if (rura[i].pozycja == 4) rura[i].pozycja = 0;
				obrot--;
			}
		}
	}

}

//Procedura rysuj¹ca jakiœ obiekt. Ustawia odpowiednie parametry dla vertex shadera i rysuje.
void drawObject() {
	//W³¹czenie programu cieniuj¹cego, który ma zostaæ u¿yty do rysowania
	//W tym programie wystarczy³oby wywo³aæ to raz, w setupShaders, ale chodzi o pokazanie, 
	//¿e mozna zmieniaæ program cieniuj¹cy podczas rysowania jednej sceny
	shaderProgram->use();
	
	//Przeka¿ do shadera macierze P,V i M.
	//W linijkach poni¿ej, polecenie:
	//  shaderProgram->getUniformLocation("P") 
	//pobiera numer slotu odpowiadaj¹cego zmiennej jednorodnej o podanej nazwie
	//UWAGA! "P" w powy¿szym poleceniu odpowiada deklaracji "uniform mat4 P;" w vertex shaderze, 
	//a matP w glm::value_ptr(matP) odpowiada deklaracji  "glm::mat4 matP;" TYM pliku.
	//Ca³a poni¿sza linijka przekazuje do zmiennej jednorodnej P w vertex shaderze dane ze zmiennej matP
	//zadeklarowanej globalnie w tym pliku. 
	//Pozosta³e polecenia dzia³aj¹ podobnie.
	matM=glm::rotate(matM,angle,glm::vec3(0,1,0));
	matM=glm::translate(matM, glm::vec3(0,0,(speedtank*move)/100));
	
	
	glUniformMatrix4fv(shaderProgram->getUniformLocation("P"),1, false, glm::value_ptr(matP));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("V"),1, false, glm::value_ptr(matV));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(matM));
	glUniform1i(shaderProgram->getUniformLocation("textureMap0"),0);
	

	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(glm::scale(glm::mat4(1), glm::vec3(40.5))));
	glBindTexture(GL_TEXTURE_2D, tex0[0]);
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[0]);
	//Narysowanie obiektu
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[0]);


	float x = -17.25;
	float z = +17.25;
	int k = 0;
	
	for (int j = 0; j < 7; j++)
	{
		for (int i = 0; i < 7; i++)
		{
			/*
			modele = glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(0, 0, 0, 1));
			rura[k].wsp_x = x;
			rura[k].wsp_z = z;
			modele = glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(x, 1, z, 1));*/
			if (rura[k].obrot == 1) {
				model[k] = glm::rotate(model[k], 90.0f, glm::vec3(0, 1, 0));
				rura[k].obrot = 0;
				printf("jestem tu\n");
			}
			
			//glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(glm::scale(glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(x, 1, z, 1)), glm::vec3(1))));
			glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(glm::scale(model[k], glm::vec3(1))));

			glBindVertexArray(vao[rura[k].model]);
			if(k==numer_pola){ glBindTexture(GL_TEXTURE_2D, tex0[0]); }
			else	glBindTexture(GL_TEXTURE_2D, tex0[rura[k].model]);
			glDrawArrays(GL_TRIANGLES, 0, vertexCount[rura[k].model]);

			k++;
			x += 5.75;
		}

		z -= 5.75;
		x = -17.25;
	}

	//los();



//////////////////////////////////////////////////////////////RURY OSOBNO///////////////////////////////////////////////////////////
/*
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(glm::scale(glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(17.25, 1, 17.25, 1)), glm::vec3(1))));
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[1]);
	glBindTexture(GL_TEXTURE_2D, tex0[1]);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[1]);
	

	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(glm::scale(glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(5.75, 1, 0, 1)), glm::vec3(1.0f))));
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[2]);
	glBindTexture(GL_TEXTURE_2D, tex0[2]);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[2]);

	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(glm::scale(glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(-5.75, 1, 0, 1)), glm::vec3(1.0f))));
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[3]);
	glBindTexture(GL_TEXTURE_2D, tex0[3]);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[3]);

	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(glm::scale(glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(11.5, 1, 17.25, 1)), glm::vec3(1.0f))));
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[4]);
	glBindTexture(GL_TEXTURE_2D, tex0[4]);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[4]);

	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, glm::value_ptr(glm::scale(glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(17.25, 1, 11.5, 1)), glm::vec3(1.0f))));
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[5]);
	glBindTexture(GL_TEXTURE_2D, tex0[5]);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount[5]);
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
	matGrass=glm::scale(glm::mat4(glm::vec4(1,0,0,0),glm::vec4(0,1,0,0),glm::vec4(0,0,1,0),glm::vec4(-30,0,-38,1)),glm::vec3(0.3));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(matGrass));
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[3]);
	glBindTexture(GL_TEXTURE_2D,tex0[3]);
	glDrawArrays(GL_TRIANGLES,0,vertexCount[3]);

	matTree=glm::scale(glm::mat4(glm::vec4(1,0,0,0),glm::vec4(0,1,0,0),glm::vec4(0,0,1,0),glm::vec4(-30,0,-38,1)),glm::vec3(0.1038f));
	//matM=glm::scale(matTree,vec3(0.1,0.1,0.1);

	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(matTree));
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[2]);
	glBindTexture(GL_TEXTURE_2D,tex0[2]);
	glDrawArrays(GL_TRIANGLES,0,vertexCount[2]);
	

	

	matCastle1=glm::scale(glm::mat4(glm::vec4(1,0,0,0),glm::vec4(0,1,0,0),glm::vec4(0,0,1,0),glm::vec4(-23,0,-40,1)),glm::vec3(0.08f));
	//matM=glm::scale(matTree,vec3(0.1,0.1,0.1);
	matCastle1=glm::rotate(matCastle1,1.57f,glm::vec3(0,1,0));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(matCastle1));
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[4]);
	glBindTexture(GL_TEXTURE_2D,tex0[4]);
	glDrawArrays(GL_TRIANGLES,0,vertexCount[4]);
	
	matCastle2=glm::scale(glm::mat4(glm::vec4(1,0,0,0),glm::vec4(0,1,0,0),glm::vec4(0,0,1,0),glm::vec4(0,-4,-50,1)),glm::vec3(0.07f));
	
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(matCastle2));
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[5]);
	glBindTexture(GL_TEXTURE_2D,tex0[5]);
	glDrawArrays(GL_TRIANGLES,0,vertexCount[5]);

	//matSky=glm::scale(glm::mat4(glm::vec4(1,0,0,0),glm::vec4(0,1,0,0),glm::vec4(0,0,1,0),glm::vec4(0,0,0,1)),glm::vec3(3));
	//glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(glm::scale(glm::mat4(1),glm::vec3(5.0))));
	matSky=glm::scale(glm::mat4(glm::vec4(1,0,0,0),glm::vec4(0,1,0,0),glm::vec4(0,0,1,0),glm::vec4(-55,-1,-55,1)),glm::vec3(100.0f));
	//matSky=glm::scale(matSky,glm::vec3(4.0));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(matSky));
	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao[6]);
	glBindTexture(GL_TEXTURE_2D,tex0[6]);
	glDrawArrays(GL_TRIANGLES,0,vertexCount[6]);
	*/
	
										
	
}

//Procedura rysuj¹ca
void displayFrame() {
	


			//Wyczyœæ bufor kolorów i bufor g³êbokoœci
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Wylicz macierz rzutowania
	matP=glm::perspective(cameraAngle, (float)Width/(float)Height, 1.0f, 100.0f);
	
	//Wylicz macierz widoku
	anglecam=anglecam-angle;
	
	glm::vec4 vectorlook;
	glm::vec3 cameralook;
	glm::vec4 vectortank;

	vectortank=matM*glm::vec4(0,0,1,0);
	if(keyStates['w'] == false && speedtank<0)  speedtank++;
	if(keyStates['s'] == false && speedtank>0)  speedtank--;

	float pi=22/7;
	vectorlook=matM*glm::vec4(0,0,0,1);


	//xcam=vectorlook.x+15*sin(vectortank.x)*cos(vectortank.z);
	//ycam=7;
	//zcam=vectorlook.z+15*cos(vectortank.x)*sin(vectortank.z);

	switch (camera){
		case 1: 
	xcam=vectorlook.x+(vectortank.x*12);
	ycam=5;
	zcam=vectorlook.z+(vectortank.z*12);
			break;

		case 2:
	xcam=vectorlook.x+15*sin(vectortank.x)*cos(vectortank.z);
	ycam=7;
	zcam=vectorlook.z+15*cos(vectortank.x)*sin(vectortank.z);
	break;

		case 3:
			xcam = 50;
			ycam = 30;
			zcam = 0;
			break;

		case 4:
			xcam= 30;
			ycam=50;
			zcam=0;
			break;

			case 5:
			xcam=40;
			ycam=10;
			zcam=vectorlook.z+1;
			break;


	}

	
	matV=glm::lookAt(glm::vec3(xcam,ycam,zcam),glm::vec3(vectorlook.x,vectorlook.y,vectorlook.z),glm::vec3(0.0f,1.0f,0.0f));
	//matV=glm::lookAt(glm::vec3(xcam,5.0f,zcam),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f)); 
	//matV=glm::translate(matV, glm::vec3(0,0,move));
	
	wektor1=matM*glm::vec4(1,0,0,0);
	wektor2=matM*glm::vec4(0,1,0,0);
	wektor3=matM*glm::vec4(0,0,1,0);
	wektor4=matM*glm::vec4(0,0,0,1);

	//printf("x=%f y=%f z=%f w=%f\n", wektor1.x, wektor1.y, wektor1.z, wektor1.w);
	//printf("x=%f y=%f z=%f w=%f\n", wektor2.x, wektor2.y, wektor2.z, wektor2.w);
	//printf("x=%f y=%f z=%f w=%f\n", wektor3.x, wektor3.y, wektor3.z, wektor3.w);
	//printf("x=%f y=%f z=%f w=%f\n\n\n", wektor4.x, wektor4.y, wektor4.z, wektor4.w);
	
	

	//Narysuj obiekt
	drawObject();
	
	//Tylny bufor na przedni
	glutSwapBuffers();
	


}

GLuint makeBuffer(std::vector<glm::vec4> data) {
	GLuint handle;

	glGenBuffers(1,&handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który bêdzie zawiera³ tablicê danych
	glBindBuffer(GL_ARRAY_BUFFER,handle);  //Uaktywnij wygenerowany uchwyt VBO 
	//glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablicê do VBO
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec4), &data[0], GL_STATIC_DRAW);
	return handle;
}

GLuint makeBuffer(std::vector<glm::vec2> data) {
	GLuint handle;

	glGenBuffers(1,&handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który bêdzie zawiera³ tablicê danych
	glBindBuffer(GL_ARRAY_BUFFER,handle);  //Uaktywnij wygenerowany uchwyt VBO 
	//glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablicê do VBO
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec2), &data[0], GL_STATIC_DRAW);
	return handle;
}

GLuint makeBuffer(void *data, int vertexCount, int vertexSize) {
	GLuint handle;
	
	glGenBuffers(1,&handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który bêdzie zawiera³ tablicê danych
	glBindBuffer(GL_ARRAY_BUFFER,handle);  //Uaktywnij wygenerowany uchwyt VBO 
	glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablicê do VBO
	
	return handle;
}

//Procedura tworz¹ca bufory VBO zawieraj¹ce dane z tablic opisuj¹cych rysowany obiekt.
void setupVBO() {
	for(int i=0;i<LICZBAELEMENTOW;i++){
		bufVertices[i]=makeBuffer(vertys[i]);
		bufColors[i]=makeBuffer(vertys[i]);
		bufNormals[i]=makeBuffer(nrmals[i]);
		bufTexCoords[i]=makeBuffer(uvs[i]);

	}
}

void assignVBOtoAttribute(char* attributeName, GLuint bufVBO, int variableSize) {
	GLuint location=shaderProgram->getAttribLocation(attributeName); //Pobierz numery slotów dla atrybutu
	glBindBuffer(GL_ARRAY_BUFFER,bufVBO);  //Uaktywnij uchwyt VBO 
	glEnableVertexAttribArray(location); //W³¹cz u¿ywanie atrybutu o numerze slotu zapisanym w zmiennej location
	glVertexAttribPointer(location,variableSize,GL_FLOAT, GL_FALSE, 0, NULL); //Dane do slotu location maj¹ byæ brane z aktywnego VBO
}

void assignVBOtoAttribute(ShaderProgram *shader,char* attributeName, GLuint bufVBO, int variableSize) {
	GLuint location=shader->getAttribLocation(attributeName); //Pobierz numery slotów dla atrybutu
	glBindBuffer(GL_ARRAY_BUFFER,bufVBO);  //Uaktywnij uchwyt VBO 
	glEnableVertexAttribArray(location); //W³¹cz u¿ywanie atrybutu o numerze slotu zapisanym w zmiennej location
	glVertexAttribPointer(location,variableSize,GL_FLOAT, GL_FALSE, 0, NULL); //Dane do slotu location maj¹ byæ brane z aktywnego VBO (null oznacza aktywny VBO)
}

//Procedura tworz¹ca VAO - "obiekt" OpenGL wi¹¿¹cy numery slotów atrybutów z buforami VBO
void setupVAO() {
	for(int i=0;i<LICZBAELEMENTOW;i++)
	{
		//Wygeneruj uchwyt na VAO i zapisz go do zmiennej globalnej
		glGenVertexArrays(1,&vao[i]);

		//Uaktywnij nowo utworzony VAO
		glBindVertexArray(vao[i]);

		assignVBOtoAttribute(shaderProgram,"vertex",bufVertices[i],4); //"vertex" odnosi siê do deklaracji "in vec4 vertex;" w vertex shaderze
		assignVBOtoAttribute(shaderProgram,"color",bufColors[i],4); //"color" odnosi siê do deklaracji "in vec4 color;" w vertex shaderze
		assignVBOtoAttribute(shaderProgram,"normal",bufNormals[i],4); //"normal" odnosi siê do deklaracji "in vec4 normal;" w vertex shaderze
		assignVBOtoAttribute(shaderProgram,"texCoord",bufTexCoords[i],2);


		//assignVBOtoAttribute(shaderProgram3,"vertex",bufVertices[i],4); //"vertex" odnosi siê do deklaracji "in vec4 vertex;" w vertex shaderze
		//assignVBOtoAttribute(shaderProgram3,"color",bufVertices[i],4); //"color" odnosi siê do deklaracji "in vec4 color;" w vertex shaderze
	}
	glBindVertexArray(0);
}

//Procedura uruchamiana okresowo. Robi animacjê.
void nextFrame(void) {
	int actTime=glutGet(GLUT_ELAPSED_TIME);
	int interval=actTime-lastTime;
	lastTime=actTime;
	//angle+=speed*interval/1000.0;
	//if (angle>360) angle-=360;
	glutPostRedisplay();
}

bool loadOBJ(const char * path, std::vector<glm::vec4> & out_vertices, std::vector<glm::vec2> & out_uvs,std::vector<glm::vec4> & out_normals){
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec4> temp_vertices; 
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec4> temp_normals;


	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file ! Are you in the right path ?\n");
		return false;
	}

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec4 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			vertex.w=1.0f;
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec4 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			normal.w=0.0f;
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				//	return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec4 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec2 uv = temp_uvs[ uvIndex-1 ];
		glm::vec4 normal = temp_normals[ normalIndex-1 ];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);

	}

	return true;
}


//Procedura wywo³ywana przy zmianie rozmiaru okna
void changeSize(int w, int h) {
	//Ustawienie wymiarow przestrzeni okna
	glViewport(0,0,w,h);
	//Zapamiêtanie nowych wymiarów okna dla poprawnego wyliczania macierzy rzutowania
	Width=w;
	Height=h;
}

//Procedura inicjuj¹ca biblotekê glut
void initGLUT(int *argc, char** argv) {
	glutInit(argc,argv); //Zainicjuj bibliotekê GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); //Alokuj bufory kolorów (podwójne buforowanie) i bufor kolorów
	
	glutInitWindowPosition(windowPositionX,windowPositionY); //Wska¿ pocz¹tkow¹ pozycjê okna
	glutInitWindowSize(Width,Height); //Wska¿ pocz¹tkowy rozmiar okna
	glutCreateWindow("Pipe"); //Utwórz okno i nadaj mu tytu³
	
	glutReshapeFunc(changeSize); //Zarejestruj procedurê changeSize jako procedurê obs³uguj¹ca zmianê rozmiaru okna
	glutDisplayFunc(displayFrame); //Zarejestruj procedurê displayFrame jako procedurê obs³uguj¹ca odœwierzanie okna
	glutIdleFunc(nextFrame); //Zarejestruj procedurê nextFrame jako procedurê wywo³ywan¹ najczêœciêj jak siê da (animacja)
}


//Procedura inicjuj¹ca bibliotekê glew
void initGLEW() {
	GLenum err=glewInit();
	if (GLEW_OK!=err) {
		fprintf(stderr,"%s\n",glewGetErrorString(err));
		exit(1);
	}
	
}



//Wczytuje vertex shader i fragment shader i ³¹czy je w program cieniuj¹cy
void setupShaders() {
	shaderProgram=new ShaderProgram("vshader.txt",NULL,"fshader.txt");
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
	for(int i=0;i<LICZBAELEMENTOW;i++){
		glDeleteBuffers(1,&bufVertices[i]);
		glDeleteBuffers(1,&bufColors[i]);
		glDeleteBuffers(1,&bufNormals[i]);
	}
}

void freeVAO() {
	for(int i=0;i<LICZBAELEMENTOW;i++){
		glDeleteVertexArrays(1,&vao[i]);
	}
}

GLuint readTexture(char* filename) {
	GLuint tex;
	TGAImg img;
	glActiveTexture(GL_TEXTURE0); 
	if (img.Load(filename)==IMG_OK) {
		glGenTextures(1,&tex); //Zainicjuj uchwyt tex
		glBindTexture(GL_TEXTURE_2D,tex); //Przetwarzaj uchwyt tex
		if (img.GetBPP()==24) //Obrazek 24bit
			glTexImage2D(GL_TEXTURE_2D,0,3,img.GetWidth(),img.GetHeight(),0,
			GL_RGB,GL_UNSIGNED_BYTE,img.GetImg());
		else if (img.GetBPP()==32) //Obrazek 32bit
			glTexImage2D(GL_TEXTURE_2D,0,4,img.GetWidth(),img.GetHeight(),0,
			GL_RGBA,GL_UNSIGNED_BYTE,img.GetImg());
		else {
			printf("Nieobs³ugiwany format obrazka w pliku: %s \n",filename);
		}
	} else {
		printf("B³¹d przy wczytywaniu pliku: %s\n",filename);
	}
	//MIPMAPPING
	glGenerateMipmap(GL_TEXTURE_2D);//wygenerowanie mipmapy
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);//uzywa dla najblizszego sasiedztwa najnizszy poziom mipmap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);

	return tex;
} 




void keyFunc(){
 
	if(keyStates['1']==true) camera=1;
	if(keyStates['2']==true) camera=2;
	if(keyStates['3']==true) camera=3;
	if(keyStates['4']==true) camera=4;
	if(keyStates['5']==true) camera=5;
	if (keyStates['9'] == true) {
		rura[numer_pola].obrot = 1;
		printf("%d zmienna\n", rura[numer_pola].obrot);
	}

    
	if(keyStates['w']==true){
		if(speedtank<1000)speedtank--;

	}
	if(keyStates['s']==true){
		if(speedtank>-1000)speedtank++;
	}
	if(keyStates['a']==true){
		if(speedtank<=0)angle=0.23;
		if(speedtank>0)angle=-0.23;
	}
	
	if(keyStates['d']==true){
		if(speedtank<=0) angle=-0.23;
		if(speedtank>0) angle=0.23;
	}
}

void keyPressed (unsigned char key, int x, int y) {
std::clog << "Wcisnieto klawisz: " << (char)key <<" kod "<< (int)key << "\n";
keyStates[key] = true;
keyFunc();
}


void keyUpFunc(unsigned char key, int x, int y){
 
    switch(key) {
    default:
        std::clog << "Zwoniono klawisz " << (char)key <<" kod "<< (int)key << "\n";
		
		keyStates[key] = false;
			
			
			//if(keyStates['s'] == false) speedtank--;
		


		
		if(keyStates['a'] == false || keyStates['d'] == false) angle=0;
        break;
    
	}
}
void SpecialInput(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		if (numer_pola > 6) numer_pola -= 7;
		else numer_pola = numer_pola + 49 - 7;
		break;
	case GLUT_KEY_RIGHT:
		if (numer_pola < 42) numer_pola += 7;
		else numer_pola = numer_pola + 7 - 49;
		break;
	case GLUT_KEY_DOWN:
		if (numer_pola % 7 != 6) numer_pola += 1;
		else numer_pola = numer_pola - 6;
		break;
	case GLUT_KEY_UP:
		if (numer_pola % 7 != 0) numer_pola -= 1;
		else numer_pola = numer_pola + 6;
		break;
	}
}
void inicjalizacja()
{
	for (int i = 0; i < 49; i++)
	{

		if (i == 18 || i == 29 || i == 38)
		{
			rura[i].model = 5;
		} else if (i == 0 || i == 24 || i == 40)
			{
				rura[i].model = 1;
			}else if (i == 8 || i == 16 || i == 19 || i == 21 || i == 28 || i == 33 || i == 45)
				{
					rura[i].model = 4;
				}else if (i == 4 || i == 5 || i == 9 || i == 17 || i == 23 || i == 27 || i == 31 || i == 35 || i == 36 || i == 46 || i == 47)
					{
						rura[i].model = 3;
				}
				else rura[i].model = 2;

		if (i == 7 || i == 10 || i == 25 || i == 40 || i == 42 || i == 44)
		{
			rura[i].pozycja = 3;
		}else if (i == 0 || i == 1 || i == 3 || i == 8 || i == 12 || i == 14 || i == 24 || i == 32 || i == 37)
			{
				rura[i].pozycja = 2;
			}else if (i == 2 || i == 4 || i == 5 || i == 6 || i == 11 || i == 17 || i == 19 || i == 20 || i == 22 || i == 26 || i == 33 || i == 41 || i == 45 || i == 46 || i == 47)
				{
					rura[i].pozycja = 1;
				}
				else rura[i].pozycja = 0;

	}
	for (int i = 0; i < LICZBAELEMENTOW; i++) {
		rura[i].obrot = 0;
	}

	float x = -17.25;
	float z = +17.25;
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

		z -= 5.75;
		x = -17.25;
	}

}



int main(int argc, char** argv) {
	//inicjalizacja(tab);

	inicjalizacja();
	
	loadOBJ("plane.obj", vertys[0], uvs[0], nrmals[0]);
	vertexCount[0]=vertys[0].size();
	loadOBJ("1_out_pipe.obj", vertys[1], uvs[1], nrmals[1]);
	vertexCount[1]=vertys[1].size();
	loadOBJ("2_out_broken_pipe.obj", vertys[2], uvs[2], nrmals[2]);
	vertexCount[2] = vertys[2].size();
	loadOBJ("2_out_connected_pipe.obj", vertys[3], uvs[3], nrmals[3]);
	vertexCount[3] = vertys[3].size();
	loadOBJ("3_out_connected_pipe.obj", vertys[4], uvs[4], nrmals[4]);
	vertexCount[4] = vertys[4].size();
	loadOBJ("4_out_pipe.obj", vertys[5], uvs[5], nrmals[5]);
	vertexCount[5] = vertys[5].size();
	
	initGLUT(&argc,argv);
	initGLEW();
	initOpenGL();

	
	
	glutKeyboardFunc(keyPressed);
	//glutKeyboardFunc(keyFunc);
	glutKeyboardUpFunc(keyUpFunc);
	glutSpecialFunc(SpecialInput);

	


	//char* textureName[LICZBAELEMENTOW] = { "niebo.tga","grass.tga","grass.tga","grass.tga","grass.tga","grass.tga" };
	tex0[0] = readTexture("6.tga");
	//tex0[LICZBAELEMENTOW] = readTexture("7.tga");
	for(int i = 1; i < LICZBAELEMENTOW; i++)
	{	       
		tex0[i] = readTexture("11.tga");	     //textureName[i]);
	}

	
	glutMainLoop();
	freeVAO();
	freeVBO();
	cleanShaders();
	return 0;
}
