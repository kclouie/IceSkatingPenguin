/* 
Final Project 

Winter 2017 - ZJW (Piddington texture write)
Look for "TODO" in this file and write new shaders
*/

#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "GLTextureWriter.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> prog, tex_prog;
shared_ptr<Shape> shape;
shared_ptr<Shape> cube;

int g_width = 512;
int g_height = 512;
float sTheta;
int gMat = 1;
int FirstTime = 1;
float Lx = 1, Ly = 1, Lz = 1;

//global reference to texture FBO
GLuint frameBuf[2];
GLuint texBuf[2];
GLuint depthBuf;

//geometry for texture render
GLuint quad_VertexArrayID;
GLuint quad_vertexbuffer;

//forward declaring a useful function listed later
void SetMaterial(int i);

/*
Helper function to create the framebuffer object and associated texture to write to
*/
void createFBO(GLuint& fb, GLuint& tex) {
  //initialize FBO (global memory)
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

	//set up framebuffer
   glBindFramebuffer(GL_FRAMEBUFFER, fb);
   //set up texture
   glBindTexture(GL_TEXTURE_2D, tex);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

   if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
     cout << "Error setting up frame buffer - exiting" << endl;
    exit(0);
	}
}

/**** geometry set up for a quad *****/
void initQuad() {

 	//now set up a simple quad for rendering FBO
  	glGenVertexArrays(1, &quad_VertexArrayID);
  	glBindVertexArray(quad_VertexArrayID);

  	static const GLfloat g_quad_vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
  	};

  	glGenBuffers(1, &quad_vertexbuffer);
  	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

}

/* lots of initialization to set up the opengl state and data */
static void initGL()
{
	GLSL::checkVersion();
  	int width, height;
  	glfwGetFramebufferSize(window, &width, &height);

	sTheta = 0;
	// Set background color.
	glClearColor(.12f, .34f, .56f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Initialize the obj mesh VBOs etc
	shape = make_shared<Shape>();
	shape->loadMesh(RESOURCE_DIR + "penguinsphere.obj");
	shape->resize();
	shape->init();

	cube = make_shared<Shape>();
	cube->loadMesh(RESOURCE_DIR + "cube.obj");
	cube->resize();
	cube->init();

	//Initialize the geometry to render a quad to the screen
	initQuad();

	// Initialize the GLSL program to render the obj
	prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	prog->init();
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addUniform("LPos");
	prog->addUniform("MatAmb");
	prog->addUniform("MatDif");
	prog->addUniform("MatSpec");
	prog->addUniform("shine");
	prog->addAttribute("vertPos");
	prog->addAttribute("vertNor");

	//create two frame buffer objects to toggle between
  	glGenFramebuffers(2, frameBuf);
  	glGenTextures(2, texBuf);
	glGenRenderbuffers(1, &depthBuf);
//	createFBO(frameBuf[0], texBuf[0]);

  //set up depth necessary since we are rendering a mesh that needs depth test
  glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
  	
	//more FBO set up
//	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
//  	glDrawBuffers(1, DrawBuffers);

	//create another FBO so we can swap back and forth
//	createFBO(frameBuf[1], texBuf[1]);
	//this one doesn't need depth

	//set up the shaders to blur the FBO decomposed just a placeholder pass thru now
	//TODO - modify and possibly add other shaders to complete blur
	tex_prog = make_shared<Program>();
	tex_prog->setVerbose(true);
	tex_prog->setShaderNames(RESOURCE_DIR + "pass_vert.glsl", RESOURCE_DIR + "tex_frag.glsl");
	tex_prog->init();
	tex_prog->addUniform("texBuf");
	tex_prog->addAttribute("vertPos");

}

/* To call the blur on the specificed texture */
/* TODO: fill in with call to appropriate shader(s) to complete the blur  */
void Blur(GLuint inTex) {
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, inTex);
   
	// example applying of 'drawing' the FBO texture
   tex_prog->bind();
   glUniform1i(tex_prog->getUniform("texBuf"), 0);
   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
   glDrawArrays(GL_TRIANGLES, 0, 6);
   glDisableVertexAttribArray(0);
   tex_prog->unbind();
  
}

/* The render loop - this function is called repeatedly during the OGL program run */
static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	
	//set up to render to buffer
//	glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[0]);
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Leave this code to just draw the meshes alone */
	//Use the matrix stack for Lab 6
   float aspect = width/(float)height;

   // Create the matrix stacks - please leave these alone for now
   auto P = make_shared<MatrixStack>();
   auto MV = make_shared<MatrixStack>();
   // Apply perspective projection.
   P->pushMatrix();
   P->perspective(45.0f, aspect, 0.01f, 100.0f);

	// Draw our scene - two meshes
	prog->bind();
	glUniform3f(prog->getUniform("LPos"), Lx, Ly, Lz);
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));

	//globl transforms for 'camera'
   	MV->pushMatrix();
    	MV->loadIdentity();
	 	MV->translate(vec3(0, 0, -5));
	 	MV->rotate(radians(-sTheta), vec3(0, 1, 0));
	  	/* Body */	
		MV->pushMatrix();
  			/* Left Eye */
			MV->pushMatrix();
				MV->translate(vec3(-.2, .3, .85));
				MV->scale(vec3(.1, .1, .1));
			  	SetMaterial(1);
		  	  	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()));
		  	  	shape->draw(prog);
			MV->popMatrix();

			/* Right Eye */
			MV->pushMatrix();
				MV->translate(vec3(.2, .3, .85));
				MV->scale(vec3(.1, .1, .1));
			  	SetMaterial(1);
		  	  	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()));
		  	  	shape->draw(prog);
			MV->popMatrix();

			/* Nose */
			MV->pushMatrix();
				MV->translate(vec3(0, .1, .5));
				MV->scale(vec3(.1, .1, .75));
			  	SetMaterial(2);
		  	  	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()));
		  	  	shape->draw(prog);
			MV->popMatrix();

			/* Left Arm */
			MV->pushMatrix();
				MV->translate(vec3(-1, -.45, .2));
				MV->rotate(-.1, vec3(0, 0, 1));
				MV->scale(vec3(.18, .9, .5));
			  	SetMaterial(1);
		  	  	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()));
		  	  	shape->draw(prog);
			MV->popMatrix();

			/* Right Arm */
			MV->pushMatrix();
				MV->translate(vec3(1, -.45, .2));
				MV->rotate(.1, vec3(0, 0, 1));
				MV->scale(vec3(.18, .9, .5));
			  	SetMaterial(1);
		  	  	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()));
		  	  	shape->draw(prog);
			MV->popMatrix();

			/* Left Foot */
			MV->pushMatrix();
				MV->translate(vec3(-.35, -1.4, .05));
				MV->rotate(-.15, vec3(0, 1, 0));
				MV->scale(vec3(.25, .10, .49));
			  	SetMaterial(2);
		  	  	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()));
		  	  	cube->draw(prog);
			MV->popMatrix();

			/* Right Foot */
			MV->pushMatrix();
				MV->translate(vec3(.35, -1.4, .05));
				MV->rotate(.15, vec3(0, 1, 0));
				MV->scale(vec3(.25, .10, .49));
			  	SetMaterial(2);
		  	  	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()));
		  	  	cube->draw(prog);
			MV->popMatrix();

			/* Body */
		  	MV->scale(vec3(1, 1.4, 1));
		  	SetMaterial(0);
	  	  	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
	  	  	shape->draw(prog);
   		MV->popMatrix();
 
   	MV->popMatrix();
   	P->popMatrix();

	prog->unbind();

}

/* helper function */
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

/* key callback */
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	} else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		gMat = (gMat+1)%4;
	} else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		sTheta += 5;
	} else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		sTheta -= 5;
	}

	
	if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		Lx-=.5;
	}
	else if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		Lx+=.5;
	}
	else if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		Ly+=.5;
	}
	else if (key == GLFW_KEY_K && action == GLFW_PRESS) {
		Ly-=.5;
	}
	
}

/* resize window call back */
static void resize_callback(GLFWwindow *window, int width, int height) {
   g_width = width;
   g_height = height;
   glViewport(0, 0, width, height);
}

//helper function to set materials for shadin
void SetMaterial(int i) {
  switch (i) {
    case 0: //shiny blue plastic
 		glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
 		glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
 		glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8); 
 		glUniform1f(prog->getUniform("shine"), 120.0);
      break;
    case 1: // flat grey
 		glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
 		glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);
 		glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4); 
 		glUniform1f(prog->getUniform("shine"), 4.0);
      break;
    case 2: //brass
 		glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
 		glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
 		glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784); 
 		glUniform1f(prog->getUniform("shine"), 27.9);
      break;
	 case 3: //copper
 		glUniform3f(prog->getUniform("MatAmb"), 0.1913, 0.0735, 0.0225);
 		glUniform3f(prog->getUniform("MatDif"), 0.7038, 0.27048, 0.0828);
 		glUniform3f(prog->getUniform("MatSpec"), 0.257, 0.1376, 0.08601); 
 		glUniform1f(prog->getUniform("shine"), 12.8);
      break;
	}
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
   //request the highest possible version of OGL - important for mac
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(g_width, g_height, "Ice Skating Penguin", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	//weird bootstrap of glGetError
   glGetError();
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
   cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
   //set the window resize call back
   glfwSetFramebufferSizeCallback(window, resize_callback);

	// Initialize scene. Note geometry initialized in init now
	initGL();

	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
