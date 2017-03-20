/* 
Final Project - Lab 8? base

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

#define TRUE 1	
#define FALSE 0

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> prog, tex_prog, prog0;

shared_ptr<Texture> texture0;

shared_ptr<Shape> shape;
shared_ptr<Shape> cube;
shared_ptr<Shape> bunny;
shared_ptr<Shape> tree;
shared_ptr<Shape> dog;

double randPos[4] = {
	-5, -35,
	-5.001, -35
};

int restrict = TRUE;
int introCount = 0;
int stepCycle;

int g_width = 600;
int g_height = 512;
float sTheta;
int gMat = 1;
int FirstTime = 1;
int firstPass;
float Lx = 50, Ly = 50, Lz = 30;

double oldx, oldy;
/*double phi, theta;*/
/*vec3 eye, LA, up;*/
/*vec3 LA;*/

double phi = 0;
double theta = -3.14 / 2;
vec3 LA = vec3(0, 0, -15);
vec3 eye = vec3(0, 0, 0);
vec3 up = vec3(0, 1, 0);

double xMove, zMove;
double lWing, lWingy, lWingx;
double rWing, rWingy, rWingx;

float pTheta;	// Rotates entire penguin 
float rfTheta; 	// Rotates penguin's right foot
float lfTheta;	// Rotates penguin's left foot
float faceTheta;

float xExpand, yExpand;
int deflate;

int lWingIn = FALSE, lWingOut = FALSE;
int rWingIn = FALSE, rWingOut = FALSE;

float pHeight;	// Raise penguin

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
	pTheta = 90;
	rfTheta = 0;
	lfTheta = 0;
	faceTheta = 0;

	xMove = -25;
	zMove = -40;
	lWing = .1;
	lWingy = 0;
	lWingx = 0;
	rWing = -.1;
	rWingy = 0;
	rWingx = 0;

	stepCycle = 0;

	pHeight = 0;
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

	bunny = make_shared<Shape>();
	bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunny->resize();
	bunny->init();

	tree = make_shared<Shape>();
	tree->loadMesh(RESOURCE_DIR + "lowpolytree.obj");
	tree->resize();
	tree->init();

	//Initialize the geometry to render a quad to the screen
	initQuad();

	// Initialize the GLSL program to render the obj
	prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	prog->init();
	prog->addUniform("P");
	prog->addUniform("M");
	prog->addUniform("V");
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

	//set up the shaders to blur the FBO decomposed just a placeholder pass thru now
	//TODO - modify and possibly add other shaders to complete blur
	tex_prog = make_shared<Program>();
	tex_prog->setVerbose(true);
	tex_prog->setShaderNames(RESOURCE_DIR + "pass_vert.glsl", RESOURCE_DIR + "tex_frag.glsl");
	tex_prog->init();
	tex_prog->addUniform("texBuf");
	tex_prog->addAttribute("vertPos");

	dog = make_shared<Shape>();
   	dog->loadMesh(RESOURCE_DIR + "cube2.obj");
   	dog->resize();
   	dog->init();

	prog0 = make_shared<Program>();
	prog0->setVerbose(true);
	prog0->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag0.glsl");
	prog0->init();

	texture0 = make_shared<Texture>();
   	texture0->setFilename(RESOURCE_DIR + "snow_1.jpg");
   	texture0->init();
   	texture0->setUnit(0);
   	texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

   	prog0->addUniform("P2");
	prog0->addUniform("MV");
	prog0->addAttribute("vertPos");
   	prog0->addAttribute("vertNor");
	prog0->addAttribute("vertTex");
   	prog0->addUniform("Texture0");
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
	
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Leave this code to just draw the meshes alone */
	//Use the matrix stack for Lab 6
   	float aspect = width/(float)height;

   	// Create the matrix stacks - please leave these alone for now
   	auto P = make_shared<MatrixStack>();
   	auto M = make_shared<MatrixStack>();
   	auto V = make_shared<MatrixStack>();
   	// Apply perspective projection.
   	P->pushMatrix();
   	/*P->perspective(45.0f, aspect, 0.01f, 100.0f);*/
   	P->perspective(45.0f, aspect, 0.01f, 100.0f);

	// Draw our scene - two meshes
	prog->bind();
	glUniform3f(prog->getUniform("LPos"), Lx, Ly, Lz);
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));

	/*if (firstPass == TRUE) {
		eye = vec3(0, 0, 0);
		LA = vec3(0, 0, -15);
		firstPass = FALSE;
	}
	else {
		LA = vec3((cos(glm::radians(phi)) * cos(glm::radians(theta))),
		  (sin(glm::radians(phi))),
		  (cos(glm::radians(phi)) * sin(glm::radians(theta))));
	}
*/
	LA = vec3((cos((phi)) * cos((theta))),
		  (sin((phi))),
		  (cos((phi)) * cos(3.14 / 2 - (theta))));

	V->pushMatrix();
	V->lookAt(eye, eye + LA, up);
	/*glm::mat4 V = lookAt(vec3(0,0,0), LA, up);*/
	glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE,value_ptr(V->topMatrix()));

	/* Draw Trees */
	/*M->pushMatrix();
		M->loadIdentity();
		for (int i = 0; i < 1; i++){
			M->translate(vec3(randPos[i * 3 + 0], 2, randPos[i * 3 + 1]));
			M->scale(vec3(3, 3, 3));
			SetMaterial(4);
  			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()));
  			tree->draw(prog);
		}
	M->popMatrix();
*/
	/* Draw Skater */
   	M->pushMatrix();
    	M->loadIdentity();
	 	M->translate(vec3(xMove, pHeight, zMove));
	 	/*M->translate(vec3(LA.x + xMove + 5, LA.y + pHeight, LA.z));*/
	 	M->rotate(radians(pTheta), vec3(0, 1, 0));
	 	M->scale(vec3(xExpand, yExpand, 1));	// Set Breathing
	  	/* Body */	
		M->pushMatrix();
  			/* Left Eye */
			M->pushMatrix();
				M->rotate(radians(faceTheta), vec3(0, 1, 0));
				M->translate(vec3(-.2, .3, .85));
				M->scale(vec3(.1, .1, .1));
			  	SetMaterial(1);
		  	  	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()));
		  	  	shape->draw(prog);
			M->popMatrix();

			/* Right Eye */
			M->pushMatrix();
				M->rotate(radians(faceTheta), vec3(0, 1, 0));
				M->translate(vec3(.2, .3, .85));
				M->scale(vec3(.1, .1, .1));
			  	SetMaterial(1);
		  	  	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()));
		  	  	shape->draw(prog);
			M->popMatrix();

			/* Nose */
			M->pushMatrix();
				M->rotate(radians(faceTheta), vec3(0, 1, 0));
				M->translate(vec3(0, .1, .5));
				M->scale(vec3(.1, .1, .75));
			  	SetMaterial(2);
		  	  	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()));
		  	  	shape->draw(prog);
			M->popMatrix();

			/* Right Wing */
			M->pushMatrix();
				M->translate(vec3(-.9, .2, 0));
				M->rotate(rWingx, vec3(1, 0, 0));
				M->rotate(rWingy, vec3(0, 1, 0));
				M->rotate(rWing, vec3(0, 0, 1));
				M->translate(vec3(0, -.7, 0));
				M->scale(vec3(.18, .9, .5));
			  	SetMaterial(1);
		  	  	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()));
		  	  	shape->draw(prog);
			M->popMatrix();

			/* Left Wing */
			M->pushMatrix();
				M->translate(vec3(.9, .2, 0));
				M->rotate(lWingx, vec3(1, 0, 0));
				M->rotate(lWingy, vec3(0, 1, 0));
				M->rotate(lWing, vec3(0, 0, 1));
				M->translate(vec3(0, -.7, 0));
				M->scale(vec3(.18, .9, .5));
			  	SetMaterial(1);
		  	  	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()));
		  	  	shape->draw(prog);
			M->popMatrix();

			/* Right Foot */
			M->pushMatrix();
				M->rotate(-rfTheta, vec3(1, 0, 0));
				M->rotate(-.15, vec3(0, .8, 0));
				M->translate(vec3(-.35, -1.4, .0));
				M->scale(vec3(.25, .10, .49));
			  	SetMaterial(2);
		  	  	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()));
		  	  	cube->draw(prog);
			M->popMatrix();

			/* Left Foot */
			M->pushMatrix();
				M->rotate(-lfTheta, vec3(1, 0, 0));
				M->rotate(.15, vec3(0, 1, 0));
				M->translate(vec3(.35, -1.4, .0));
				M->scale(vec3(.25, .10, .49));
			  	SetMaterial(2);
		  	  	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()));
		  	  	cube->draw(prog);
			M->popMatrix();

			/* Body */
		  	M->scale(vec3(1, 1.4, 1));
		  	SetMaterial(0);
	  	  	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()) );
	  	  	shape->draw(prog);
   		M->popMatrix();
   	M->popMatrix();

   	/* Draw Trees */
	M->pushMatrix();
		M->loadIdentity();
		for (int i = 0; i < 2; i++){
			M->pushMatrix();
				M->translate(vec3(randPos[i * 3 + 0], 1, randPos[i * 3 + 1]));
				M->scale(vec3(3, 3, 3));
				SetMaterial(4);
	  			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()));
	  			tree->draw(prog);
	  		M->popMatrix();
		}
	M->popMatrix();

   	M->pushMatrix();
   		M->translate(vec3(0, -2, 0));
		M->scale(vec3(500, .1, 500));
	  	SetMaterial(1);
  	  	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()));
  	  	cube->draw(prog);
   	M->popMatrix();

   	P->popMatrix();

	prog->unbind();

	/*auto MV = make_shared<MatrixStack>();
	auto P2 = make_shared<MatrixStack>();
	//draw the dog mesh 	
	prog0->bind();
   	texture0->bind(prog0->getUniform("Texture0"));
	glUniformMatrix4fv(prog0->getUniform("P2"), 1, GL_FALSE, value_ptr(P->topMatrix()));

	MV->pushMatrix();
		MV->loadIdentity();	
		MV->pushMatrix();
		    MV->translate(vec3(0, 0.5, -.5));
		    MV->scale(vec3(.5, .5, .5));	
			glUniformMatrix4fv(prog0->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
		   	dog->draw(prog0);
		MV->popMatrix();
	MV->popMatrix();
	prog0->unbind();*/

	// Breathing 
	if (xExpand < 1.04 && deflate == FALSE) {
		xExpand += .0009;
	}
	else {
		deflate = TRUE;
		xExpand -= .0009;
		if (xExpand == 1) {
			deflate = FALSE;
		}
	}

	/////////////////////////////////////////////////////////
	//////////// Begin Intro Animation Sequence ////////////
	/////////////////////////////////////////////////////////
	if (restrict) {
		// 1. Enter
		if (introCount == 0) {
			xMove += .15; 
			if (xMove > -10) {	
				xMove = -10;
				introCount++;
			}
			if (rWingx > -.75) {
				rWingx -= .01;
				rWing += .003;
			}
			if (lWingx > -.75) {
				lWingx -= .01;
				lWing -= .003;
			}
			if (stepCycle % 2 == 0) {
				if (lfTheta > -1) lfTheta -= .03;	// Step Right
				if (rfTheta < 0) rfTheta += .1;
				if (lfTheta <= -1) {
					lfTheta = -1;
					stepCycle++;
				}
			}
			if (stepCycle % 2 != 0) {
				if (rfTheta > -1) rfTheta -= .03;	// Step Left
				if (lfTheta < 0) lfTheta += .1;
				if (rfTheta <= -1) {
					rfTheta = -1;
					stepCycle++;
				}
			}
		}

		// 2. Present
		else if (introCount == 1) {
			zMove += .15;
			if (pTheta > 0) pTheta -= 3;
			if (zMove > -25) {
				zMove = -25;
				introCount++;
			}
			if (rWingy > -3.75) {
				rWingy -= .05;
			}
			if (lWingy < 3.75) {
				lWingy += .05;
			}

			if (stepCycle % 2 == 0) {
				if (lfTheta > -1) lfTheta -= .03;	// Step Right
				if (rfTheta < 0) rfTheta += .1;
				if (lfTheta <= -1) {
					lfTheta = -1;
					stepCycle++;
				}
			}
			if (stepCycle % 2 != 0) {
				if (rfTheta > -1) rfTheta -= .03;	// Step Left
				if (lfTheta < 0) lfTheta += .1;
				if (rfTheta <= -1) {
					rfTheta = -1;
					stepCycle++;
				}
			}
		}

		// 3. Glide stage left
		else if (introCount == 2) {
			if (pTheta < 90) pTheta += 2;
			if (xMove < 6) xMove += .1;
			if (xMove > 6) {
				xMove = 6;
				introCount++;
			}
			if (rWingy < 0) rWingy += .1;
			if (rWingx < 0) rWingx += .1;
			if (rWing > -.1) rWing -= .1;	// Re-init right wing

			if (lWingy > 0) lWingy -= .5;
			if (lWingx < 0) lWingx += .5;
			if (lWing < 3) lWing += .1;	// Raise left wing

			if (lfTheta < 0) lfTheta += .1;		// Lower Right Foot
			if (rfTheta > -1) rfTheta -= .02;		// Lift Left Foot
		}

		// 4. Glide stage right
		else if (introCount == 3) {
			if (pTheta > -90) pTheta -= 8;
			if (xMove > -8) xMove -= .1;
			if (xMove < -8) {
				xMove = -8;
				introCount++;
			}
			if (rWing > -3) rWing -= .1;	// Raise right wing
			if (lWing > .1) lWing -= .2;

			if (lfTheta > -1) lfTheta -= .02;	// Lift Right Foot
			if (rfTheta < 0) rfTheta += .1;	// Lower Left Foot
		}

		// 5. Grounded Outward Spin
		else if (introCount == 4) {
			pTheta -= 10;
			if (pTheta <= -990 && introCount == 4) {
				pTheta = 90;
				introCount++;
			}
			if (rWing > -3.5) rWing -= .05;
			if (lWing < 3.5) lWing += .15;

			/*if (rfTheta < 0) rfTheta += .2;	// Lower Left Foot
			if (lfTheta < 0) lfTheta += .2;	// Lower Left Foot*/

			if (lfTheta < 0) lfTheta += .2;
			else lfTheta = 0;
			rfTheta = 0;

		}
		 
		// 6. Prep stage left & take off
		else if (introCount == 5) {
			if (xMove < -2) xMove += .1;
			if (pTheta > 32) pTheta -= 8;
			if (xMove > -2) {
				xMove += .5;
				pHeight -= .25;
				introCount++;
			}
			if (rWing < -.75) rWing += .1;
			if (lWing > 2) lWing -= .1;

			if (stepCycle % 2 == 0) {
				if (lfTheta > -1) lfTheta -= .03;	// Step Right
				if (rfTheta < 0) rfTheta += .1;
				if (lfTheta <= -1) {
					lfTheta = -1;
					stepCycle++;
				}
			}
			if (stepCycle % 2 != 0) {
				if (rfTheta > -1) rfTheta -= .03;	// Step Left
				if (lfTheta < 0) lfTheta += .1;
				if (rfTheta <= -1) {
					rfTheta = -1;
					stepCycle++;
				}
			}
		}

		// 7. Rise 
		else if (introCount == 6) {
			if (pHeight < 1.75) {
				pHeight += .1;
				xMove += .02;
				/*introCount++;*/
			}
			else {
				pHeight = 1.75;
				introCount++;
			}
			if (rWing > -1.75) rWing -= .1;

			if (rfTheta < 0) rfTheta += .5;
			if (lfTheta < 0) lfTheta += .5;
		}

		// 8. Come down
		else if (introCount == 7) {
			if (pHeight > 0) {
				pHeight -= .1;
				xMove += .02;
			}
			else {
				pHeight = 0;
				introCount++;
			}
			if (rWing < -.5) rWing += .05;
			if (lWing > .5) lWing -= .05;
		}
		

		// 9. Land, prep again stage left, take off
		else if (introCount == 8) {
			/*if (pTheta < 90) pTheta += 3;*/
			if (xMove < 2) xMove += .1;
			if (xMove > 2) {
				xMove = 2;
				pHeight -= .25;
				introCount++;
			}
			if (rWing < -.5) rWing += .05;	// Lower arms
			if (lWing > .5) lWing -= .05;

			if (stepCycle % 2 == 0) {
				if (lfTheta > -1) lfTheta -= .03;	// Step Right
				if (rfTheta < 0) rfTheta += .1;
				if (lfTheta <= -1) {
					lfTheta = -1;
					stepCycle++;
				}
			}
			if (stepCycle % 2 != 0) {
				if (rfTheta > -1) rfTheta -= .03;	// Step Left
				if (lfTheta < 0) lfTheta += .1;
				if (rfTheta <= -1) {
					rfTheta = -1;
					stepCycle++;
				}
			}
		}

		// 10. Rise & spin
		else if (introCount == 9) {
			if (pHeight < 1.75) {
				pHeight += .1;
				xMove += .02;
				/*introCount++;*/
			}
			else {
				pHeight = 1.75; 
				pTheta -= 10;
				if (pTheta <= -360) {
					pTheta = 0;
					introCount++;
				}
			}
			if (rWing > -3) rWing -= .1;
			if (lWing < 3) lWing += .1;
		}

		// 11. Come down
		else if (introCount == 10) {
			if (pHeight > 0) {
				pHeight -= .1;
				xMove += .02;
			}
			else {
				pHeight = 0;
				introCount++;
			}
			if (rWing < 0) rWing += .15;
			if (lWing > 0) lWing -= .15;
		}

		// 12. Take center
		else if (introCount == 11) {
			if (xMove > 0) xMove -= .1;
			if (zMove < -15) zMove += .06;


			if (xMove <= 0 && zMove >= -15) {
				faceTheta = 0;	// END!

				lWing = .1;
				lWingy = 0;
				lWingx = 0;
				rWing = -.1;
				rWingy = 0;
				rWingx = 0;

				lfTheta = 0;
				rfTheta = 0;

				pHeight = 0;
				restrict = FALSE;
			}


			if (rWingx > -3) {	// Raise arms
				rWingx -= .03;
			}
			if (lWingx > -3) {
				lWingx -= .03;
			}
			if (rWingx <= -3 && lWingx <= -3) {
				if (rWing > -3) rWing -= .08;	// Lower arms
				if (lWing < 3) lWing += .08;
			}

			if (stepCycle % 2 == 0) {
				if (lfTheta > -1) lfTheta -= .03;	// Step Right
				if (rfTheta < 0) rfTheta += .1;
				if (lfTheta <= -1) {
					lfTheta = -1;
					stepCycle++;
				}
			}
			if (stepCycle % 2 != 0) {
				if (rfTheta > -1) rfTheta -= .03;	// Step Left
				if (lfTheta < 0) lfTheta += .1;
				if (rfTheta <= -1) {
					rfTheta = -1;
					stepCycle++;
				}
			}
		}
	}	// End restrict

}

/* helper function */
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}


/* key callback */
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	/*float speed = .1;*/
	float speed = .25;

	if (restrict) {
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			restrict = FALSE;
		}

		else if(key == GLFW_KEY_ESCAPE && action == GLFW_REPEAT) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}
	else {
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		/* Move Right */
		else if (key == GLFW_KEY_A) {
			xMove -= .2;
			faceTheta = 0;

			/* Rotate penguin slight left */
			if (pTheta > -45) pTheta -= 8;

			/* Control Right Wing */
			if (rWing < 0 && rWingOut == TRUE) rWing += .4; // Return to first from Low
			else {
				rWingOut = FALSE;				// Advance to High
				if (rWing > -1.75) {
					rWing -= .3;
					rWingIn = TRUE;
				}
			}

			/* Control Left Wing */
			if (lWing > 0 && lWingIn == TRUE) lWing -= .4;	// Return to first from High
			else {
				lWingIn = FALSE;				// Advance to Low
				if (lWing <= .75){
					lWing += .15;
					lWingOut = TRUE;
				}
			}
	 
			if (lfTheta > -1) lfTheta -= .25;	// Lift Right Foot
			if (rfTheta < 0) rfTheta += .25;	// Lower Left Foot
		}

		/* Move Left */
		else if (key == GLFW_KEY_D) {
			xMove += .2;
			faceTheta = 0;

			/* Rotate penguin slight right*/
			if (pTheta < 45) pTheta += 8;

			/* Control Right Wing */
			if (rWing < 0 && rWingIn == TRUE) rWing += .4;	// Return to first from High
			else {
				rWingIn = FALSE;				// Advance to Low
				if (rWing > -.75) {
					rWing -= .15;
					rWingOut = TRUE;
				}
			}

			/* Control Left Wing */
			if (lWing > 0 && lWingOut == TRUE) lWing -= .2;	// Return to first from Low
			else {
				lWingOut = FALSE;					// Advance to High
				if (lWing <= 1.75){
					lWing += .3;
					lWingIn = TRUE;
				}
			}

			if (lfTheta < 0) lfTheta += .25;		// Lower Right Foot
			if (rfTheta > -1) rfTheta -= .25;		// Lift Left Foot
		}
		// ---------------------------------------

		/* Skate backwards */
		else if (key == GLFW_KEY_W) {
			zMove -= .4;
			if (faceTheta < 32) faceTheta += 8;
			if (pTheta < 0) pTheta += 8;
			else if (pTheta > 0) pTheta -= 8;
			if (pTheta == 0) {
				if (rWing > -1.5) rWing -= .3;
				if (lWing < 1.5) lWing += .3;

				if (rfTheta < 0) rfTheta += .3;
				if (lfTheta < 0) lfTheta += .3;
			}
		}

		/* Skate forward */
		else if (key == GLFW_KEY_S) {
			zMove += .4;
			faceTheta = 0;

			if (pTheta < 0) pTheta += 8;
			else if (pTheta > 0) pTheta -= 8;
			if (pTheta == 0) {
				if (rWing > -1.5) rWing -= .3;
				if (lWing < 1.5) lWing += .3;

				if (lfTheta < 0) lfTheta += .25;		// Lower Right Foot
				if (rfTheta > -1) rfTheta -= .1;		// Lift Left Foot
			}
		}

		/* Grounded Left Turn */
		else if (key == GLFW_KEY_Q && action == GLFW_REPEAT) {
			faceTheta = 0;

			rfTheta = 0;
			lfTheta = 0;
			lWing = -3;
			rWing = -1.5;
			if (pTheta < 0) pTheta -= 25;
			else pTheta += 25;
		}

		/* Left Turn Recovery */
		else if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
			faceTheta = 0;

			pTheta = 0;
			lfTheta = 0;
			rfTheta = 0;
			lWing = 0;
			rWing = 0;
		}

		/* Grounded Right Turn */
		else if (key == GLFW_KEY_E && action == GLFW_REPEAT) {
			faceTheta = 0;

			rfTheta = 0;
			lfTheta = 0;
			lWing = 1.5;
			rWing = -3;
			if (pTheta < 0) pTheta -= 25;
			else pTheta += 25;
		}

		/* Right Turn Recovery */
		else if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
			faceTheta = 0;

			pTheta = 0;
			lfTheta = 0;
			rfTheta = 0;
			lWing = 0;
			rWing = 0;
		}

		/* Regular Jump */
		else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			faceTheta = 0;

			pHeight = 1.75;
			lfTheta = -.5;
			rfTheta = -.5;
			lWing = -4;
			rWing = 4;
		}

		/* Spinning Jump */
		else if (key == GLFW_KEY_SPACE && action == GLFW_REPEAT) {
			faceTheta = 0;

			pHeight = 2;
			if (pTheta < 0) pTheta -= 25;
			else pTheta += 25;
		}

		/* Landing */
		else if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			faceTheta = 0;
			pHeight = 0;
			pTheta = 0;
			lfTheta = 0;
			rfTheta = 0;
			lWing = 0;
			rWing = 0;
		}


		else if (key == GLFW_KEY_UP) {	// forward
			eye += speed * LA;
		}
		else if (key == GLFW_KEY_DOWN) {	// back
			eye -= speed * LA;
		}
		else if (key == GLFW_KEY_LEFT) {	// left
			eye -= normalize(cross(LA, up)) * speed;
		}
		else if (key == GLFW_KEY_RIGHT) {	// right
			eye += normalize(cross(LA, up)) * speed;
		}
		/*} else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
			gMat = (gMat+1)%4;
		}*/ else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			sTheta += 5;
		} else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			sTheta -= 5;
		}	
	}
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	double deltax, deltay;
	int width, height;
  	glfwGetFramebufferSize(window, &width, &height);

  	if (restrict) return;
  	else {
  		deltax = xpos - oldx;
		deltay = ypos - oldy;
		/*theta += deltax * 3.14159265359 / height;
		phi += deltay * 3.14159265359 / width;*/
		theta += deltax * .001;
		phi += deltay * .001;

		if (phi >= glm::radians(80.0)) {
			phi = glm::radians(80.0);
		}
		if (phi <= glm::radians(-80.0)) {
			phi = glm::radians(-80.0);
		}

		oldx = xpos;
		oldy = ypos;
  	}

	/*LA = normalize(vec3((cos(glm::radians(phi)) * cos(glm::radians(theta))),
		  (sin(glm::radians(phi))),
		  (cos(glm::radians(phi)) * sin(glm::radians(theta)))));*/
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
	    case 0: // shiny blue plastic
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
	    case 2: // brass
	 		glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
	 		glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
	 		glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784); 
	 		glUniform1f(prog->getUniform("shine"), 27.9);
	      	break;
		case 3: // copper
	 		glUniform3f(prog->getUniform("MatAmb"), 0.1913, 0.0735, 0.0225);
	 		glUniform3f(prog->getUniform("MatDif"), 0.7038, 0.27048, 0.0828);
	 		glUniform3f(prog->getUniform("MatSpec"), 0.257, 0.1376, 0.08601); 
	 		glUniform1f(prog->getUniform("shine"), 12.8);
	      	break;
	    case 4: // tree green
	 		glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
	 		glUniform3f(prog->getUniform("MatDif"), 0.0, 0.9, 0.16);
	 		glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8); 
	 		glUniform1f(prog->getUniform("shine"), 120.0);
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, g_width/2.0, g_height/2.0);
	glfwGetCursorPos(window, &oldx, &oldy);
	glfwSetCursorPosCallback(window, cursor_position_callback);
   	//set the window resize call back
   	glfwSetFramebufferSizeCallback(window, resize_callback);

	// Initialize scene. Note geometry initialized in init now
   	/*for (int i = 0; i < 20; i++) {
   		randPos[i * 3 + 0] = rand() % (10 - 10 + 1) + 10;	// randX
	 	randPos[i * 3 + 1] = rand() % (-6 + 7 + 1) - 7;		// randZ
	 }*/

	 firstPass = TRUE;
	 xExpand = 1;
	 yExpand = 1;
	 deflate = FALSE;

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
