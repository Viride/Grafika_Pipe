/*
Niniejszy program jest wolnym oprogramowaniem; mo�esz go
rozprowadza� dalej i / lub modyfikowa� na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundacj� Wolnego
Oprogramowania - wed�ug wersji 2 tej Licencji lub(wed�ug twojego
wyboru) kt�rej� z p�niejszych wersji.

Niniejszy program rozpowszechniany jest z nadziej�, i� b�dzie on
u�yteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domy�lnej
gwarancji PRZYDATNO�CI HANDLOWEJ albo PRZYDATNO�CI DO OKRE�LONYCH
ZASTOSOWA�.W celu uzyskania bli�szych informacji si�gnij do
Powszechnej Licencji Publicznej GNU.

Z pewno�ci� wraz z niniejszym programem otrzyma�e� te� egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
je�li nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#ifndef TEAPOT_H
#define TEAPOT_H

//Utah teapot model made out of triangles
//Contains arrays
//vertices - vertex positions in homogenous coordinates
//normals -vertex normals in homogenous coordinates (flat shading)
//vertexNormals - vertex normals in homogenous coordinates (smooth shading)
//texCoords -  texturing coordinates
//colors - vertex colors (rgba)
//c1, c2, c3 - kolejne kolumny odwr�conej macierzy TBN
//c1_2, c2_2, c3_2 - kolejne kolumny odwr�conej i u�rednionej macierzy TBN
//TBN friendly
//Culling GL_CW

#include "model.h"

namespace Models {
		
	
	namespace TeapotInternal {
		extern float vertices[];
		extern float normals[];
		extern float vertexNormals[];
		extern float texCoords[];
		extern float colors[];
		extern float c1[];
		extern float c2[];
		extern float c3[];
		extern float c1_2[];
		extern float c2_2[];
		extern float c3_2[];
		extern unsigned int vertexCount;
	} 
 
	class Teapot: public Model {
		public:
			Teapot();			
			virtual ~Teapot();
			virtual void drawSolid();			
	};

	extern Teapot teapot;

}
#endif
