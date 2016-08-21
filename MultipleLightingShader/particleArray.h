#pragma once
#include "rt3d.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>

class particleArray
{
private:
	int numParticles;
	glm::vec3* positions;
	glm::vec3* colours;
	glm::vec3* vel;
	glm::vec3* vel2;	// for easy manipulation for smoke effects
	glm::vec3* vel3;	// for easy manipulation for smoke effects
	GLuint vao[1];
	GLuint vbo[2];
	float d_time, n_time, l_time, c_time, life_time, grav, alpha;
	bool created, blend;
public:
	particleArray(const int n, bool c);
	~particleArray();
	int getNumParticles() const { return numParticles; }
	glm::vec3* getPositions() const { return positions; }
	glm::vec3* getColours() const { return colours; }
	glm::vec3* getVel() const { return vel; }
	float getLifeTime() { return life_time; }
	void update();
	void draw();
	float getAlphafade() { return alpha; }
	void setAlphafade(float a) { alpha = a; }
};