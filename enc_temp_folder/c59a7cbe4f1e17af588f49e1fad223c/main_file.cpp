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
//TODO: dodać oznaczenie twórców modeli 3D

#include "links.h"
#define GLM_FORCE_RADIANS

using namespace glm;
using namespace std;

float aspectRatio = 1; //do skalowania okna programu
float speed_x = 0; //[radiany/s]
float speed_y = 0; //[radiany/s]
float walk_speed = 0;
//TODO: dostosować prędkość do realnych sekund/minut
float speed = 0.1f;
//TODO: dodać prędkość przyspieszoną do prezentacji działania mechanizmu
//...

vec3 pos = glm::vec3(0, 2, -11);

//STEP: Kamera
vec3 calcDir(float kat_x, float kat_y) {
	vec4 dir = vec4(0, 0, 1, 0);
	mat4 M = rotate(glm::mat4(1.0f), kat_y, vec3(0, 1, 0));
	M = rotate(M, kat_x, vec3(1, 0, 0));
	dir = M * dir;
	return vec3(dir);
}

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
	
		for (int i = 0; i < mesh->mNumVertices; i++) {
			aiVector3D vertex = mesh->mVertices[i]; //aiVector3D podobny do glm::vec3
			this->verts.push_back(vec4(vertex.x, vertex.y, vertex.z, 1));

			aiVector3D normal = mesh->mNormals[i]; //Wektory znormalizowane
			this->norms.push_back(vec4(normal.x, normal.y, normal.z, 0));
		
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


Object* gear1;
Object* gear2;
Object* pudlo;
Object* moon;
Object* pendulum;
Object* face;
Object* glass;
Object* mWskazowka;
Object* dWskazowka;
Object* dyngs;
Object* roomFloor;
Object* roomWall;

//STEP: Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//STEP: Sterowanie
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_y = 1;
		if (key == GLFW_KEY_RIGHT) speed_y = -1;
		if (key == GLFW_KEY_M) speed_x = 1;
		if (key == GLFW_KEY_N) speed_x = -1;
		if (key == GLFW_KEY_UP) walk_speed = 5;
		if (key == GLFW_KEY_DOWN) walk_speed = -5;
	}

	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_y = 0;
		if (key == GLFW_KEY_RIGHT) speed_y = 0;
		if (key == GLFW_KEY_M) speed_x = 0;
		if (key == GLFW_KEY_N) speed_x = 0;
		if (key == GLFW_KEY_UP) walk_speed = 0;
		if (key == GLFW_KEY_DOWN) walk_speed = 0;
	}
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

//STEP: Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	initShaders();	
	glClearColor(0.f, 0.f, 0.5f, 0.f); //Ustaw kolor czyszczenia bufora kolorów
	glEnable(GL_DEPTH_TEST); //Włącz test głębokości na pikselach
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, key_callback); //włączenie sterowania
	gear1 = new Object(string("gears.obj"), "wahadlo.png");
	gear2 = new Object(string("gear2.obj"), "wahadlo.png");
	pudlo = new Object(string("zegar.obj"), "pudlo.png");
	moon = new Object(string("moon.obj"), "moon.png");
	pendulum = new Object(string("wahadlo.obj"), "wahadlo.png");
	face = new Object(string("face.obj"), "tarcza.png");
	//dWskazowka = new Object(string("dWskazowka.obj"), "dwskazowka.png");
	//mWskazowka = new Object(string("mWskazowka.obj"), "steel.png");
	dyngs = new Object(string("dyngs.obj"), "steel.png");
	roomFloor = new Object(string("floor.obj"), "floor.png");
	roomWall = new Object(string("floor.obj"), "wall.png");

	//TODO: jak wyrenderować szkło?
	//glass = new Object(string(""), "");
}

//STEP: Procedura zwalniania zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	freeShaders();
	glDeleteTextures(1, &(gear1->tex));
	delete gear1;
	glDeleteTextures(1, &(gear2->tex));
	delete gear2;
	glDeleteTextures(1, &(pudlo->tex));
	delete pudlo;
	glDeleteTextures(1, &(moon->tex));
	delete moon;
	glDeleteTextures(1, &(pendulum->tex));
	delete pendulum;
	glDeleteTextures(1, &(face->tex));
	delete face;
	/*glDeleteTextures(1, &(dWskazowka->tex));
	delete dWskazowka;
	glDeleteTextures(1, &(mWskazowka->tex));
	delete mWskazowka;*/
	glDeleteTextures(1, &(dyngs->tex));
	delete dyngs;
	glDeleteTextures(1, &(roomFloor->tex));
	delete roomFloor;
	glDeleteTextures(1, &(roomWall->tex));
	delete roomWall;
	
	//glDeleteTextures(1, &(glass->tex));
	//delete glass;
}


//STEP: Procedura rysująca obiekty (spLambertTexture)
void drawObject(Object* object, mat4 objectMatrix/*, mat4 viewMatrix, mat4 perspectiveMatrix*/) {

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
void drawScene(GLFWwindow* window, float kat_x,float kat_y, float angle) {
	using namespace Models;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości
		
	spLambertTextured->use();	

	glEnableVertexAttribArray(spLambertTextured->a("vertex"));
	glEnableVertexAttribArray(spLambertTextured->a("normal"));
	glEnableVertexAttribArray(spLambertTextured->a("texCoord"));

	//OBIEKTY DYNAMICZNE
	//gear1
	mat4 Mgear1 = mat4(1.f);
	Mgear1 = translate(Mgear1, vec3(0.f, 3.05f, 8.f));
	Mgear1 = scale(Mgear1, vec3(0.7f, 0.7f, 0.7f));
	Mgear1 = rotate(Mgear1, angle, vec3(0.0f, 0.0f, 1.0f));
	
	//gear2
	mat4 Mgear2 = mat4(0.4f);
	Mgear2 = translate(Mgear2, vec3(0.f, 2.65f, 8.f));
	Mgear2 = scale(Mgear2, vec3(0.7f, 0.7f, 0.7f));
	Mgear2 = rotate(Mgear2, (1.1f*angle), vec3(0.0f, 0.0f, -1.0f));

	//TODO: obrót wahadła zależny od pełnej h
	//TODO: obrót wahadła ograniczony do 30 stopni
	//wahadlo
	mat4 Mpendulum = mat4(1.f);
	Mpendulum = scale(Mpendulum, vec3(4.f, 4.f, 4.f));
	Mpendulum = translate(Mpendulum, vec3(0.f, 0.5f, 2.1f));
	/*if (pendiculumAcumulator >= 0 && pendiculumAcumulator < 10)
	{
		Mpendulum = rotate(Mpendulum, angle, vec3(0.0f, 0.0f, -1.0f));
	}
	else
	{
		Mpendulum = rotate(Mpendulum, angle, vec3(0.0f, 0.0f, 1.0f));
	}*/

	//TODO: animacja obrotu zależna od obrotu wskazówek
	//skrzynia pory dnia
	mat4 Mmoon = mat4(1.f);
	Mmoon = scale(Mmoon, vec3(4.f, 4.f, 3.67f));
	Mmoon = translate(Mmoon, vec3(0.f, 0.99f, 2.1f));
	Mmoon = rotate(Mmoon, angle, vec3(0.f, 0.f, 1.f));

	//duza wskazowka
	mat4 MduzaWskazowka = mat4(1.f);
	MduzaWskazowka = translate(MduzaWskazowka, vec3(0.75f, 0.f, 2.1));
	MduzaWskazowka = scale(MduzaWskazowka, vec3(0.2f, 0.2f, 0.2f));

	//mala wskazowka
	mat4 MmalaWskazowka = mat4(1.f);
	MmalaWskazowka = scale(MmalaWskazowka, vec3(0.1f, 0.f, 0.1f));

	//OBIEKTY STATYCZNE
	//dyngs
	mat4 Mdyngs = mat4(1.f);
	Mdyngs = translate(Mdyngs, vec3(0.f, 3.05f, 7.66f));
	Mdyngs = scale(Mdyngs, vec3(0.7f, 0.7f, 0.7f));

	//skrzynia zegara
	mat4 Mpudlo = mat4(1.f);
	Mpudlo = scale(Mpudlo, vec3(4.f, 4.f, 4.f));
	Mpudlo = translate(Mpudlo, vec3(0.f, -1.f, 2.1f));

	//tarcza zegara
	mat4 Mface = mat4(1.f);
	Mface = translate(Mface, vec3(0.f, 3.05f, 7.7f));
	Mface = scale(Mface, vec3(0.4f, 0.4f, 0.4f));

	//TODO: SZKŁO
	////szkło
	//mat4 Mglass = mat4(1.f);
	//Mglass = scale(Mglass, vec3(4.f, 4.f, 4.f));
	//Mglass = translate(Mglass, vec3(0.f, -1.f, 0.f));

	//pomieszczenie
	//podloga
	mat4 Mfloor = mat4(1.f);
	Mfloor = translate(Mfloor, vec3(0.f, -4.f, 0.f));
	//sciana lewa
	mat4 MleftWall = mat4(1.f);
	MleftWall = translate(MleftWall, vec3(10.f, 0.f, 0.f));
	MleftWall = rotate(MleftWall, 1.57f, vec3(0.f, 0.f, 1.f));
	//sciana prawa
	mat4 MrightWall = mat4(1.f);
	MrightWall = translate(MrightWall, vec3(-10.f, 0.f, 0.f));
	MrightWall = rotate(MrightWall, 1.57f, vec3(0.f, 0.f, 1.f));
	//sciana przednia
	mat4 MfrontWall = mat4(1.f);
	MfrontWall = translate(MfrontWall, vec3(0.f, 0.f, 10.f));
	MfrontWall = rotate(MfrontWall, 1.57f, vec3(1.f, 0.f, 0.f));

	//NOTE: Macierze V, P
	vec3 center = pos + calcDir(kat_x, kat_y);
	vec3 noseVector = vec3(0.0f, 1.0f, 0.0f);
	mat4 V = lookAt(pos, center, noseVector); //Wylicz macierz widoku

	float fovy = radians(50.0f);
	float zNear = 1.f;
	float zFar = 50.f;
	mat4 P = perspective(fovy, aspectRatio, zNear, zFar);

	//NOTE: rysowanie obiektów
	drawObject(gear1, Mgear1);
	drawObject(gear2, Mgear2);
	drawObject(pudlo, Mpudlo);
	drawObject(moon, Mmoon);
	drawObject(pendulum, Mpendulum);
	drawObject(face, Mface);
	//drawObject(mWskazowka, MmalaWskazowka);
	//drawObject(dWskazowka, MduzaWskazowka);
	drawObject(dyngs, Mdyngs);
	//drawObject(glass, Mglass);
	drawObject(roomFloor, Mfloor);
	drawObject(roomWall, MleftWall);
	drawObject(roomWall, MrightWall);
	drawObject(roomWall, MfrontWall);

	//wysyłanie macierzy M,V,P do GPU:
	glUniformMatrix4fv(spLambertTextured->u("V"), 1, false, value_ptr(V));
	glUniformMatrix4fv(spLambertTextured->u("P"), 1, false, value_ptr(P));
		
	glDisableVertexAttribArray(spLambertTextured->a("vertex"));
	glDisableVertexAttribArray(spLambertTextured->a("texCoord"));
	glDisableVertexAttribArray(spLambertTextured->a("normal"));

	glfwSwapBuffers(window); //Skopiuj bufor tylny do bufora przedniego
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


	window = glfwCreateWindow(1000, 1000, "ZEGAR", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.


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
	float angle = 0; //zadeklaruj zmienną przechowującą aktualny kąt obrotu
	float kat_x = 0;
	float kat_y = 0;
	glfwSetTime(0); //Wyzeruj licznik czasu
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		kat_x += speed_x * glfwGetTime();
		kat_y += speed_y * glfwGetTime();
		pos += (float)(walk_speed * glfwGetTime()) * calcDir(kat_x, kat_y);
		angle+=speed * glfwGetTime();
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window,kat_x,kat_y,angle); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);


	//NOTE: Zwalnianie zasobów
	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
