/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

/* ZEGAR - PIETRZYK, SAMELAK*/

#include "links.h"

using namespace glm;
using namespace std;

//STEP: Klasa obiektu
class Object {
public:
	GLuint tex;

	vector<vec4> verts;
	vector<vec4> norms;
	vector<vec2> texCoords;
	vector<unsigned int> indices;

	Object(string plik, const char* filename) {
		loadModel(plik);
		readTexture(filename);
	}

	~Object() {}

	//STEP: Procedura wczytywania modelu .obj
	void loadModel(string plik) {

		using namespace std;
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
		cout << importer.GetErrorString() << endl;

		aiMesh* mesh = scene->mMeshes[0];

		//NOTE: TU SIĘ SYPIE
		for (int i = 0; i < mesh->mNumVertices; i++) {
			aiVector3D vertex = mesh->mVertices[i]; //aiVector3D podobny do glm::vec3
			this->verts.push_back(vec4(vertex.x, vertex.y, vertex.z, 1));

			aiVector3D normal = mesh->mNormals[i]; //Wektory znormalizowane
			this->norms.push_back(vec4(normal.x, normal.y, normal.z, 0));

			//NOTE: coś z tym
		
			//liczba zdefiniowanych zestawów wsp. teksturowania (zestawów jest max 8)
			//unsigned int liczba_zest = mesh->GetNumColorChannels();
			//liczba składowych wsp. teksturowania dla 0 zestawu
			//unsigned int wymiar_wsp_tex = mesh->mNumUVComponents[0];
			
			//0 to numer zestawu współrzędnych teksturowania
			aiVector3D texCoord = mesh->mTextureCoords[0][i];
			this->texCoords.push_back(vec2(texCoord.x, texCoord.y));
			//x,y,z wykorzystywane jako u,v,w. 0 jeżeli tekstura ma mniej wymiarów
		}

		//dla każdego wielokąta składowego
		for (int i = 0; i < mesh->mNumFaces; i++) {
			aiFace& face = mesh->mFaces[i]; //face to jeden z wielokątów siatki

			//dla każdego indeksu->wierzchołka tworzącego wielokąt
			//dla aiProcess_Triangulate to zawsze będzie 3
			for (int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		/*for (int i = 0; i < 19; i++) {
			cout << i << " " << material->GetTextureCount((aiTextureType)i) << endl;
		}*/

		for (int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
			aiString str; //nazwa pliku

			/*aiTextureMapping mapping; //jak wygenerowano np. teksturowania (opcj.)
			unsigned int uvMapping; //numer zestawu wsp. teksturowania (opcjonalne)
			ai_real blend; //współczynnik połązenia kolorów z kolejną teksturą (op.)
			aiTextureOp op; //sposób łączenia kolorów z kolejną teksturą (opcjonalne)
			aiTextureMapMode mapMode; //sposób adresowania tekstury (opcjonalne)*/
			material->GetTexture(aiTextureType_DIFFUSE, i, &str); //, &mapping, &uvMapping, &blend, &op, &mapMode);
			cout << str.C_Str() << endl;
		}
	}

	//STEP: Procedura wczytywania tekstur
	void readTexture(const char* filename) {
		GLuint tex;
		glActiveTexture(GL_TEXTURE0);

		std::vector<unsigned char> image;
		unsigned width, height;
		unsigned error = lodepng::decode(image, width, height, filename);

		glGenTextures(1, &(this->tex));
		glBindTexture(GL_TEXTURE_2D, this->tex);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}
};


Object* cup;
Object* cubeObject;

//STEP: Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}


//STEP: Sterowanie
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
	//TODO: sterowanie
}


//STEP: Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	initShaders();	
	glEnable(GL_DEPTH_TEST); //Włącz test głębokości na pikselach
	glfwSetKeyCallback(window, key_callback); //włączenie sterowania
	cup = new Object(string("Cup.obj"), "glass.png");
	cubeObject = new Object(string("cube.obj"), "glass.png");
}

//STEP: Procedura zwalniania zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	freeShaders();
	glDeleteTextures(1, &(cup->tex));
	delete cup;
	glDeleteTextures(1, &(cubeObject->tex));
	delete cubeObject;
}


//STEP: Procedura rysująca obiekty
void drawObject(Object* object, mat4 objectMatrix, mat4 viewMatrix, mat4 perspectiveMatrix) {

	glUniformMatrix4fv(spLambertTextured->u("M"), 1, false, value_ptr(objectMatrix));

	glVertexAttribPointer(spLambertTextured->a("vertex"), 4, GL_FLOAT, false, 0, object->verts.data());
	glVertexAttribPointer(spLambertTextured->a("normal"), 4, GL_FLOAT, false, 0, object->norms.data());
	glVertexAttribPointer(spLambertTextured->a("texCoord"), 2, GL_FLOAT, false, 0, object->texCoords.data());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (object->tex));
	glUniform1i(spLambertTextured->u("tex"), 0);
	glDrawElements(GL_TRIANGLES, object->indices.size(), GL_UNSIGNED_INT, object->indices.data());
};

//STEP: Procedura rysująca scenę
void drawScene(GLFWwindow* window) {
	using namespace Models;

	glClearColor(0.f, 0.f, 1.f, 0.f); //Ustaw kolor czyszczenia bufora kolorów
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości
		
	spLambertTextured->use();	

	glEnableVertexAttribArray(spLambertTextured->a("vertex"));
	glEnableVertexAttribArray(spLambertTextured->a("normal"));
	glEnableVertexAttribArray(spLambertTextured->a("texCoord"));

	//NOTE: macierze M obiektów
	//cube
	mat4 M = mat4(1.f);
	M = translate(M, vec3(5.f, 0.f, 0.f));
	
	//cup
	mat4 Mcup = mat4(1.f);
	Mcup = translate(Mcup, vec3(-5.f, 0.f, 0.f));
	Mcup = scale(Mcup, vec3(0.5f, 0.5f, 0.5f));

	//NOTE: Macierze V, P
	vec3 observer = vec3(0.0f, 0.0f, -18.0f);
	vec3 center = vec3(0.0f, 0.0f, 0.0f);
	vec3 noseVector = vec3(0.0f, 1.0f, 0.0f);
	mat4 V = lookAt(observer, center, noseVector);

	float fovy = radians(50.0f);
	float zNear = 1.f;
	float zFar = 50.f;
	mat4 P = perspective(fovy, 1.f, zNear, zFar);


	//NOTE: rysowanie obiektów
	drawObject(cubeObject, M, V, P);
	drawObject(cup, Mcup, V, P);


	//wysyłanie macierzy M,V,P do GPU:
	glUniformMatrix4fv(spLambertTextured->u("V"), 1, false, value_ptr(V));
	glUniformMatrix4fv(spLambertTextured->u("P"), 1, false, value_ptr(P));
	
	
	glDisableVertexAttribArray(spLambertTextured->a("vertex"));
	glDisableVertexAttribArray(spLambertTextured->a("texCoord"));
	glDisableVertexAttribArray(spLambertTextured->a("normal"));


	glfwSwapBuffers(window); //Skopiuj bufor tylny do bufora przedniego
}


//STEP: Procedura rysująca obiekty
void drawObjects(GLFWwindow* window) {

	//TODO: rysowanie obiektu

}

int main(void)
{
	//NOTE: Inicjalizacja window
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(720, 720, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące


	//STEP: Główna pętla	
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{		
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);


	//NOTE: Zwalnianie zasobów
	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
