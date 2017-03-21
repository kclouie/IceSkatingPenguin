//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"

using namespace std;

float randFloat(float l, float h)
{
	float r = rand() / (float)RAND_MAX;
	return (1.0f - r) * l + r * h;
}

Particle::Particle() :
	charge(1.0f),
	m(1.0f),
	d(0.0f),
	x(0.0f, 0.0f, 0.0f),
	v(0.0f, 0.0f, 0.0f),
	lifespan(1.0f),
	tEnd(0.0f),
	scale(1.0f),
	color(1.0f, 1.0f, 1.0f, 1.0f)
{
}

Particle::~Particle()
{
}

void Particle::load()
{
	// Random initialization
	rebirth(0.0f);
}

/* all particles born at the origin */
void Particle::rebirth(float t)
{
	charge = randFloat(0.0f, 1.0f) < 0.5 ? -1.0f : 1.0f;	
	m = 1.0f;
  	d = randFloat(0.0f, 0.02f);
   x.x = randFloat(-30.0f, 30.0f);
   x.y = 12.0f;
   x.z = randFloat(-20.0f, 20.0f); 
	v.x = randFloat(-0.0f, 0.0f);
	v.y = randFloat(-1.5f, -0.1f);
	v.z = randFloat(-0.1f, 0.1f);
	lifespan = randFloat(100.0f, 200.0f); 
	tEnd = t + lifespan;
	scale = randFloat(0.2, 1.0f);
   color.r = randFloat(0.0f, 0.1f);
   color.g = randFloat(0.0f, 0.1f);
   color.b = randFloat(0.25f, 0.5f);
	color.a = 1.0f;
}

void Particle::update(float t, float h, const vec3 &g, const bool *keyToggles)
{
	/*if(t > tEnd) {
		rebirth(t);
	}*/
	if (x.y < -5) {
		rebirth(t);
	}

	//very simple update
	x += h*v; // h*v = VELOCITY
	color.a = (tEnd-t)/lifespan;
}
