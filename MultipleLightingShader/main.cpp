// MD2 animation renderer
// This demo will load and render an animated MD2 model, an OBJ model and a skybox
// Most of the OpenGL code for dealing with buffer objects, etc has been moved to a 
// utility library, to make creation and display of mesh objects as simple as possible

// Windows specific: Uncomment the following line to open a console window for debug output
#if _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#endif

#include "rt3d.h"
#include "rt3dObjLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stack>
#include <vector>

#include <SDL_ttf.h>

using namespace std;

#define DEG_TO_RADIAN 0.017453293

#define LIGHT_COUNT 3 // DEFINED AMOUNT OF LIGHTS

// Globals
// Real programs don't use globals

GLuint meshIndexCount = 0;
GLuint toonIndexCount = 0;
GLuint md2VertCount = 0;
GLuint meshObjects[3];


GLuint shaderProgram;
GLuint skyboxProgram;

glm::mat4 projection(1.0);

GLfloat r = 0.0f;

glm::vec3 eye(-2.0f, 1.0f, 8.0f);
glm::vec3 at(0.0f, 1.0f, -1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

stack<glm::mat4> mvStack;

// TEXTURE STUFF
GLuint skybox[5];

//--------------====================================================---------------------------//
//--------------===================LIGHT STUFF======================---------------------------//
//--------------====================================================---------------------------//

std::vector<rt3d::lightStruct> light(LIGHT_COUNT); // Setting up a Vector of Light structs of size LIGHT_COUNT ( In this case it is 3)
GLuint LightBuffer; // Int for the Light buffer

glm::vec4 l_Amb(0.0f, 0.0f, 0.0f, 1.0f); // Light Ambient

glm::vec4 l_DifBase(0.0f, 0.0f, 0.0f, 1.0f); // Light Diffuse Blue
glm::vec4 l_DifWhite(1.0f, 1.0f, 1.0f, 1.0f); // Light Diffuse white
glm::vec4 l_DifR(1.0f, 0.0f, 0.0f, 1.0f); // Light Diffuse Red
glm::vec4 l_DifG(0.0f, 1.0f, 0.0f, 1.0f); // Light Diffuse Green
glm::vec4 l_DifB(0.0f, 0.0f, 1.0f, 1.0f); // Light Diffuse Blue


glm::vec4 l_Spe(0.30f, 0.30f, 0.40f, 1.0f); // Light Specular

glm::vec4 l_Pos1(-5.0f, 2.0f, -1.7f, 1.0f); // Light 1 Position
glm::vec4 l_Pos2(-5.0f, 4.0f, -1.7f, 1.0f); // Light 2 Position
glm::vec4 l_Pos3(-5.0f, 0.0f, -1.7f, 1.0f); // Light 3 Position


// light attenuation
float attConstant;
float attLinear;
float attQuadratic;

// Variables needed for Light Movement
float speed;
double timer;
bool movingLeft;

// Variables needed for Light Switches
bool redLight_On[LIGHT_COUNT];
bool greenLight_On[LIGHT_COUNT];
bool blueLight_On[LIGHT_COUNT];
bool clearColour;
bool turnOnAll;
int currentLight;


//=============================================================================================//
//=============================================================================================//
//=============================================================================================//

rt3d::materialStruct material0 = {
	{0.0f, 0.0f, 0.0f, 1.0f}, // ambient
	{0.3f, 0.3f, 0.3f, 1.0f}, // diffuse
	{1.0f, 1.0f, 1.0f, 1.0f}, // specular
	20.0f  // shininess
};

float theta = 0.0f;

TTF_Font * textFont;

// textToTexture
GLuint textToTexture(const char * str, GLuint textID/*, TTF_Font *font, SDL_Color colour, GLuint &w,GLuint &h */) {
	TTF_Font *font = textFont;
	SDL_Color colour = { 255, 255, 255 };
	SDL_Color bg = { 0, 0, 0 };

	SDL_Surface *stringImage;
	stringImage = TTF_RenderText_Blended(font, str, colour);

	if (stringImage == NULL)
		//exitFatalError("String surface not created.");
		std::cout << "String surface not created." << std::endl;

	GLuint w = stringImage->w;
	GLuint h = stringImage->h;
	GLuint colours = stringImage->format->BytesPerPixel;

	GLuint format, internalFormat;
	if (colours == 4) {   // alpha
		if (stringImage->format->Rmask == 0x000000ff)
			format = GL_RGBA;
		else
			format = GL_BGRA;
	}
	else {             // no alpha
		if (stringImage->format->Rmask == 0x000000ff)
			format = GL_RGB;
		else
			format = GL_BGR;
	}
	internalFormat = (colours == 4) ? GL_RGBA : GL_RGB;

	GLuint texture = textID;

	if (texture == 0) {
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} //Do this only when you initialise the texture to avoid memory leakage

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, stringImage->w, stringImage->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, stringImage->pixels);
	glBindTexture(GL_TEXTURE_2D, NULL);

	SDL_FreeSurface(stringImage);
	return texture;
}

// Set up rendering context
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
		rt3d::exitFatalError("Unable to initialize SDL");

	// Request an OpenGL 3.0 context.

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // double buffering on
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // 8 bit alpha buffering
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Turn on x4 multisampling anti-aliasing (MSAA)

	// Create 800x600 window
	window = SDL_CreateWindow("SDL/GLM/OpenGL Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!window) // Check window was created OK
		rt3d::exitFatalError("Unable to create window");

	context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
	SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

// A simple texture loading function
// lots of room for improvement - and better error checking!
GLuint loadBitmap(char *fname) {
	GLuint texID;
	glGenTextures(1, &texID); // generate texture ID

	// load file - using core SDL library
	SDL_Surface *tmpSurface;
	tmpSurface = SDL_LoadBMP(fname);
	if (!tmpSurface) {
		std::cout << "Error loading bitmap" << std::endl;
	}

	// bind texture and set parameters
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	SDL_PixelFormat *format = tmpSurface->format;

	GLuint externalFormat, internalFormat;
	if (format->Amask) {
		internalFormat = GL_RGBA;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGBA : GL_BGRA;
	}
	else {
		internalFormat = GL_RGB;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, tmpSurface->w, tmpSurface->h, 0,
		externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(tmpSurface); // texture loaded, free the temporary buffer
	return texID;	// return value of texture ID
}

void lightInit()
{
	//--------------====================================================---------------------------//
	//--------------===================LIGHT STUFF======================---------------------------//
	//--------------====================================================---------------------------//
	for (int i = 0; i < LIGHT_COUNT; ++i) // For all the lights 
	{
		for (int j = 0; j < 4; ++j) // For all the individual floats within the vec 4
		{
			light[0].diffuse[j] = l_DifBase[j]; // set light[0]'s diffuse[0,1,2,3] to the l_DifR[0,1,2,3]
			light[1].diffuse[j] = l_DifBase[j]; // set light[1]'s diffuse[0,1,2,3] to the l_DifG[0,1,2,3]
			light[2].diffuse[j] = l_DifBase[j]; // set light[2]'s diffuse[0,1,2,3] to the l_DifB[0,1,2,3]
			light[i].ambient[j] = l_Amb[j];  // set light[0,1,2]'s ambient[0,1,2,3] to the l_Amb[0,1,2,3]
			light[i].specular[j] = l_Spe[j]; // set light[0,1,2]'s specular[0,1,2,3] to the l_Spe[0,1,2,3]
			redLight_On[i] = false;
			greenLight_On[i] = false;
			blueLight_On[i] = false;
		}
	}

	// Initialising variables needed for Light Attenuation
	attConstant = 1.0f;
	attLinear = 0.0f;
	attQuadratic = 0.0f;

	// Initialising variables needed for Light Movement
	speed = 0.05;
	timer = 30.0;
	movingLeft = true;

	// Initialising variables needed for Light Switches
	clearColour = false;
	turnOnAll = false;
	currentLight = 0;

	//-----------------------------------Positioning the Lights--------------------------------------
	light[0].position[0] = l_Pos1[0];
	light[0].position[1] = l_Pos1[1];
	light[0].position[2] = l_Pos1[2];
	light[0].position[3] = l_Pos1[3];

	light[1].position[0] = l_Pos2[0];
	light[1].position[1] = l_Pos2[1];
	light[1].position[2] = l_Pos2[2];
	light[1].position[3] = l_Pos2[3];

	light[2].position[0] = l_Pos3[0];
	light[2].position[1] = l_Pos3[1];
	light[2].position[2] = l_Pos3[2];
	light[2].position[3] = l_Pos3[3];
	//----------------------------------------------------------------------------------------------

	// set light attenuation shader uniforms
	GLuint uniformIndex = glGetUniformLocation(shaderProgram, "attConst");
	glUniform1f(uniformIndex, attConstant);
	uniformIndex = glGetUniformLocation(shaderProgram, "attLinear");
	glUniform1f(uniformIndex, attLinear);
	uniformIndex = glGetUniformLocation(shaderProgram, "attQuadratic");
	glUniform1f(uniformIndex, attQuadratic);

	//=============================================================================================//
	//=============================================================================================//
	//=============================================================================================//	
}
void bufferInit()
{
	GLuint error = 0;
	error = glGetError();

	glGenBuffers(1, &LightBuffer); //Generate the Buffers initially
	error = glGetError(); // This block below all Buffer code is for errors
	if (error != GL_NO_ERROR)
		std::cout << "Error: " << error << std::endl;

	glBindBuffer(GL_UNIFORM_BUFFER, LightBuffer); // Binding the light buffer
	error = glGetError();
	if (error != GL_NO_ERROR)
		std::cout << "Error: " << error << std::endl;

	glBufferData(GL_UNIFORM_BUFFER, sizeof(rt3d::lightStruct) * LIGHT_COUNT, NULL, GL_DYNAMIC_DRAW);//Getting the buffer data
	error = glGetError();
	if (error != GL_NO_ERROR)
		std::cout << "Error: " << error << std::endl;
}
void init(void)
{
	shaderProgram = rt3d::initShaders("phong-tex.vert", "phong-tex.frag"); // Set up the shaders

	lightInit(); // intialises all light variables
	bufferInit(); // initially sets up buffers

	vector<GLfloat> verts;
	vector<GLfloat> norms;
	vector<GLfloat> tex_coords;
	vector<GLuint> indices;
	rt3d::loadObj("cube.obj", verts, norms, tex_coords, indices);
	meshIndexCount = indices.size();
	meshObjects[0] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), tex_coords.data(), meshIndexCount, indices.data());
	

	verts.clear(); norms.clear();tex_coords.clear();indices.clear();
	rt3d::loadObj("bunny-5000.obj", verts, norms, tex_coords, indices);
	toonIndexCount = indices.size();
	meshObjects[2] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), nullptr, toonIndexCount, indices.data());

	std::cout << "Currently changing Light: " << +currentLight << std::endl;
}

glm::vec3 moveForward(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::sin(r*DEG_TO_RADIAN), pos.y, pos.z - d*std::cos(r*DEG_TO_RADIAN));
}

glm::vec3 moveRight(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::cos(r*DEG_TO_RADIAN), pos.y, pos.z + d*std::sin(r*DEG_TO_RADIAN));
}

void swingLight(void) // Method for the movement on the lights
{
	if (timer == 0.0)
		{
			movingLeft = false;
		}

	if (movingLeft == true)
	{
		
		timer -= 0.5;
		light[0].position[0] -= speed;
		light[1].position[1] += speed;
		light[2].position[2] += speed;

	}
	else if (movingLeft == false)
	{
		timer += 0.5;
		light[0].position[0] += speed;
		light[1].position[1] -= speed;
		light[2].position[2] -= speed;
		if (timer == 90.0)
		{
			movingLeft = true;
		}
	}
		
}

void lightSwitch(void) // Method for handling all the Light changes ( switches )
{
	if (redLight_On[currentLight] == true) // RED ON
	{	
		light[currentLight].diffuse[0] = 1.0f;
	}
	if (greenLight_On[currentLight] == true) // GREEN ON
	{
		light[currentLight].diffuse[1] = 1.0f;
	}
	if (blueLight_On[currentLight] == true) // BLUE ON
	{
		light[currentLight].diffuse[2] = 1.0f;
	}
	if (redLight_On[currentLight] == false) // RED OFF
	{
		light[currentLight].diffuse[0] = 0.0f;
	}
	if (greenLight_On[currentLight] == false) // GREEN OFF
	{
		light[currentLight].diffuse[1] = 0.0f;
	}
	if (blueLight_On[currentLight] == false) // BLUE OFF
	{
		light[currentLight].diffuse[2] = 0.0f;
	}
	if (clearColour == true) // RESET THE COLOURS BACK TO 0.0f
	{
		for (int j = 0; j < 3; ++j)
		{

			redLight_On[j] = false;
			greenLight_On[j] = false;
			blueLight_On[j] = false;
		}

		for (int i = 0; i < LIGHT_COUNT; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				light[i].diffuse[j] = l_DifBase[j];
			}

		}
		clearColour = false;
	}
	if (turnOnAll == true) // TURN ALL THE LIGHTS ON
	{
		for (int j = 0; j < 3; ++j)
		{
			redLight_On[j] = false;
			greenLight_On[j] = false;
			blueLight_On[j] = false;
		}

		for (int j = 0; j < 4; ++j)
		{
			light[0].diffuse[j] = l_DifR[j];
			light[1].diffuse[j] = l_DifG[j];
			light[2].diffuse[j] = l_DifB[j];

		}
	}
}
void lightUpdate(const Uint8 *keys)
{
	lightSwitch(); // Calling the handler for light colour changes
	swingLight(); // Calling the method for moving the light 

	//--------------Controls of Red Light--------------------------
	if (keys[SDL_SCANCODE_R] && redLight_On[currentLight] == false)
	{
		turnOnAll = false;
		std::cout << "\nTurning red light on at position: " << +currentLight << std::endl;
		redLight_On[currentLight] = true;
	}

	if (keys[SDL_SCANCODE_T] && redLight_On[currentLight] == true)
	{
		turnOnAll = false;
		std::cout << "\n\tTurning off red light at: " << +currentLight << std::endl;
		redLight_On[currentLight] = false;
	}
	//-------------------------------------------------------------


	//------------Controls of Green Light--------------------------
	if (keys[SDL_SCANCODE_G] && greenLight_On[currentLight] == false)
	{
		turnOnAll = false;
		std::cout << "\nTurning green light on at position: " << +currentLight << std::endl;
		greenLight_On[currentLight] = true;
	}
	if (keys[SDL_SCANCODE_H] && greenLight_On[currentLight] == true)
	{
		turnOnAll = false;
		std::cout << "\n\tTurning off green light at: " << +currentLight << std::endl;
		greenLight_On[currentLight] = false;
	}
	//-------------------------------------------------------------


	//--------------Controls of Blue Light--------------------------
	if (keys[SDL_SCANCODE_B] && blueLight_On[currentLight] == false)
	{
		turnOnAll = false;
		std::cout << "\nTurning blue light on at position: " << +currentLight << std::endl;
		blueLight_On[currentLight] = true;
	}
	if (keys[SDL_SCANCODE_N] && blueLight_On[currentLight] == true)
	{
		turnOnAll = false;
		std::cout << "\n\tTurning off blue light at: " << +currentLight << std::endl;
		blueLight_On[currentLight] = false;
	}
	//-------------------------------------------------------------
	//------------------Current Light Changer----------------------
	if (keys[SDL_SCANCODE_1])
	{
		turnOnAll = false;
		currentLight = 0;
		std::cout << "Currently changing Light: " << +currentLight << std::endl;
	}
	if (keys[SDL_SCANCODE_2])
	{
		turnOnAll = false;
		currentLight = 1;
		std::cout << "Currently changing Light: " << +currentLight << std::endl;
	}
	if (keys[SDL_SCANCODE_3])
	{
		turnOnAll = false;
		currentLight = 2;
		std::cout << "Currently changing Light: " << +currentLight << std::endl;
	}
	if (keys[SDL_SCANCODE_C])
	{
		turnOnAll = false;
		clearColour = true;
	}
	if (keys[SDL_SCANCODE_RETURN])
	{
		turnOnAll = true;
		std::cout << "\nTurning Light 1: Red\nTurning Light 2: Green\nTurning Light 3: Blue" << std::endl;
	}
	//-------------------------------------------------------------
}

void update(void) 
{
	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	lightUpdate(keys); // Calling the light update method

	//--------------------------Movement---------------------------
	if (keys[SDL_SCANCODE_W]) 
		eye = moveForward(eye, r, 0.1f);

	if (keys[SDL_SCANCODE_S]) 
		eye = moveForward(eye, r, -0.1f);

	if (keys[SDL_SCANCODE_A]) 
		eye = moveRight(eye, r, -0.1f);

	if (keys[SDL_SCANCODE_D]) 
		eye = moveRight(eye, r, 0.1f);
	//-------------------------------------------------------------

	if (keys[SDL_SCANCODE_COMMA]) //rotations
		r -= 1.0f;
	if (keys[SDL_SCANCODE_PERIOD]) //rotations
		r += 1.0f;

	if (keys[SDL_SCANCODE_ESCAPE]) //Exit program
	{
		exit(1);
	}
	if (keys[SDL_SCANCODE_L]) //Fill
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
	}
}

void lightDraw()
{
	//--------------====================================================---------------------------//
	//--------------===================LIGHT STUFF======================---------------------------//
	//--------------====================================================---------------------------//


	glUseProgram(shaderProgram); // Using the Shader program
	GLint activeBlock; // Int for Buffer methods
	GLuint b_Index = glGetUniformBlockIndex(shaderProgram, "Lights"); // creading the index for the buffers
	glGetActiveUniformBlockiv(shaderProgram, b_Index, GL_UNIFORM_BLOCK_DATA_SIZE, &activeBlock); // getting the current active buffer

	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection)); //setting the uniform matrix up for the shader program

	GLuint error = 0;
	glBindBuffer(GL_UNIFORM_BUFFER, LightBuffer); // binding the buffer
	error = glGetError();// error check
	if (error != GL_NO_ERROR)
		std::cout << "Error: " << error << std::endl;
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(rt3d::lightStruct) * LIGHT_COUNT, light.data()); // collecting the sub data
	error = glGetError();
	if (error != GL_NO_ERROR)
		std::cout << "Error: " << error << std::endl;

	glBindBufferBase(GL_UNIFORM_BUFFER, b_Index, LightBuffer); // Binding the buffer to base



	// draw a small cube block at lightPos
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(light[0].position[0], light[0].position[1], light[0].position[2]));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.15f, 0.15f, 0.15f));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(shaderProgram, material0);
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// draw a small cube block at lightPos1
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(light[1].position[0], light[1].position[1], light[1].position[2]));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.15f, 0.15f, 0.15f));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(shaderProgram, material0);
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();


	// draw a small cube block at lightPos2
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(light[2].position[0], light[2].position[1], light[2].position[2]));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.15f, 0.15f, 0.15f));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(shaderProgram, material0);
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();


	//=============================================================================================//
	//=============================================================================================//
	//=============================================================================================//	

}
void draw(SDL_Window * window) 
{
	// clear the screen
	glEnable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	
	projection = glm::perspective(float(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 150.0f);

	GLfloat scale(1.0f); // just to allow easy scaling of complete scene

	glm::mat4 modelview(1.0); // set base position for scene
	mvStack.push(modelview);

	at = moveForward(eye, r, 1.0f);
	mvStack.top() = glm::lookAt(eye, at, up);

	// back to remainder of rendering
	glDepthMask(GL_TRUE); // make sure depth test is on
							 
	lightDraw();

	// draw the toon shaded bunny
	glUseProgram(shaderProgram);
	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-4.0f, 0.1f, -2.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0, 20.0, 20.0));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(shaderProgram, material0);
	rt3d::drawIndexedMesh(meshObjects[2], toonIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// remember to use at least one pop operation per push...
	mvStack.pop(); // initial matrix
	glDepthMask(GL_TRUE);


	SDL_GL_SwapWindow(window); // swap buffers
}


// Program entry point - SDL manages the actual WinMain entry point for us
int main(int argc, char *argv[]) {
	SDL_Window * hWindow; // window handle
	SDL_GLContext glContext; // OpenGL context handle
	hWindow = setupRC(glContext); // Create window and render context 

	// Required on Windows *only* init GLEW to access OpenGL beyond 1.1
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // glewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << endl;
		exit(1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();

	bool running = true; // set running to true
	SDL_Event sdlEvent;  // variable to detect SDL events
	while (running) {	// the event loop
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT)
				running = false;
		}
		update();
		draw(hWindow); // call the draw function
	}
	glDeleteBuffers(1, &LightBuffer); // Deleting the buffers
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(hWindow);
	SDL_Quit();
	return 0;
}