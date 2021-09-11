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

GLuint tex;

vector<vec4> verts;
vector<vec4> norms;
vector<vec2> texCoords;
vector<unsigned int> indices;

//STEP: Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//STEP: Sterowanie
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
	//TODO: sterowanie
}

//STEP: Procedura wczytywania modelu .obj
void loadModel(std::string plik) {
	
	using namespace std;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	cout << importer.GetErrorString() << endl;

	aiMesh* mesh = scene->mMeshes[0];

	for (int i = 0; i < mesh->mNumVertices; i++) {
		aiVector3D vertex = mesh->mVertices[i]; //aiVector3D podobny do glm::vec3
		verts.push_back(vec4(vertex.x, vertex.y, vertex.z, 1));

		aiVector3D normal = mesh->mNormals[i]; //Wektory znormalizowane
		norms.push_back(vec4(normal.x, normal.y, normal.z, 0));

		/*
		//liczba zdefiniowanych zestawów wsp. teksturowania (zestawów jest max 8)
		unsigned int liczba_zest = mesh->GetNumColorChannels();
		//liczba składowych wsp. teksturowania dla 0 zestawu
		unsigned int wymiar_wsp_tex = mesh->mNumUVComponents[0];
		*/
		//0 to numer zestawu współrzędnych teksturowania
		aiVector3D texCoord = mesh->mTextureCoords[0][i];
		texCoords.push_back(vec2(texCoord.x, texCoord.y));
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
GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filename);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

//STEP: Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	initShaders();	
	glEnable(GL_DEPTH_TEST); //Włącz test głębokości na pikselach
	glfwSetKeyCallback(window, key_callback); //włączenie sterowania
	loadModel(string("Cup.obj"));
	tex = readTexture("bricks.png");
}

//STEP: Procedura zwalniania zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	freeShaders();
	glDeleteTextures(1, &tex);
}

//STEP: Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window) {
	glClearColor(0.f, 0.f, 1.f, 0.f); //Ustaw kolor czyszczenia bufora kolorów
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości
		
	spLambertTextured->use();

	vec3 observer = vec3(0.0f, 0.0f, -18.0f);
	vec3 center = vec3(0.0f, 0.0f, 0.0f);
	vec3 noseVector = vec3(0.0f, 1.0f, 0.0f);
	mat4 V = lookAt(observer, center, noseVector);
	//wysyłanie macierzy V do GPU:
	glUniformMatrix4fv(spLambertTextured->u("V"), 1, false, value_ptr(V));

	float fovy = radians(50.0f);
	float zNear = 1.f;
	float zFar = 50.f;
	mat4 P = perspective(fovy, 1.f, zNear, zFar);
	//wysyłanie macierzy P do GPU:
	glUniformMatrix4fv(spLambertTextured->u("P"), 1, false, value_ptr(P));

	using namespace Models;
	mat4 M = mat4(1.f);
	//wysyłanie macierzy M do GPU:
	glUniformMatrix4fv(spLambertTextured->u("M"), 1, false, value_ptr(M));
	glUniform4f(spLambertTextured->u("color"), 1.f, 0.5f, 0.f, 0.f);

	glEnableVertexAttribArray(spLambertTextured->a("vertex"));
	glVertexAttribPointer(spLambertTextured->a("vertex"), 4, GL_FLOAT, false, 0, verts.data()); 
	glEnableVertexAttribArray(spLambertTextured->a("normal"));
	glVertexAttribPointer(spLambertTextured->a("normal"), 4, GL_FLOAT, false, 0, norms.data());
	glEnableVertexAttribArray(spLambertTextured->a("texCoord"));
	glVertexAttribPointer(spLambertTextured->a("texCoord"), 2, GL_FLOAT, false, 0, texCoords.data());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(spLambertTextured->u("tex"), 0); 
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());

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

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

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
