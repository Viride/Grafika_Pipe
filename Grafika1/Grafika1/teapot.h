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

#ifndef TEAPOT_H
#define TEAPOT_H

//Utah teapot model made out of triangles
//Contains arrays
//vertices - vertex positions in homogenous coordinates
//normals -vertex normals in homogenous coordinates (flat shading)
//vertexNormals - vertex normals in homogenous coordinates (smooth shading)
//texCoords -  texturing coordinates
//colors - vertex colors (rgba)
//c1, c2, c3 - kolejne kolumny odwróconej macierzy TBN
//c1_2, c2_2, c3_2 - kolejne kolumny odwróconej i uœrednionej macierzy TBN
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
