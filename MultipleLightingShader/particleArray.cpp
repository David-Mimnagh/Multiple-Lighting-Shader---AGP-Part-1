#include "particleArray.h"

particleArray::particleArray(const int n, bool c) : numParticles(n)
{
	if (numParticles <= 0) // trap invalid input
		return;

	blend = false;
	created = c;

	positions = new glm::vec3[numParticles];
	colours = new glm::vec3[numParticles];
	vel = new glm::vec3[numParticles];
	vel2 = new glm::vec3[numParticles];
	vel3 = new glm::vec3[numParticles];

	c_time = 0;
	n_time = 0;
	l_time = 0;
	d_time = 0;
	life_time = 100.0f;
	grav = 0.9 / 5000.0f;
	alpha = 1.0f;

	// lets initialise with some lovely random values!
	std::srand(std::time(0));
	for (int i = 0; i < numParticles; i++)
	{
		vel[i].x = (GLfloat)(((std::rand() % 100) - 50) / 2500.0f);
		vel2[i].x = vel[i].x * 2;
		vel3[i].x = (GLfloat)(((std::rand() % 100) + 50) / 1000.0f);
		vel[i].y = (GLfloat)((std::rand() % 100) / 1000.0f);
		vel[i].z = (GLfloat)(((std::rand() % 100) - 50) / 2500.0f);
		vel2[i].z = vel[i].z * 2;
	}
	for (int i = 0; i < numParticles; i++)	// X coords
	{
		positions[i].x = 0; // (float)(((std::rand() % 20) - 10) / 10);
		positions[i].y = 0;	//(float)((std::rand() % 2 / 100));
		positions[i].z = 0; // (float)(((std::rand() % 20) - 10) / 10);
	}

	for (int i = 0; i < numParticles; i++)	//
	{
		colours[i] = glm::vec3(1.0f);
	}


	// Initialise VAO and VBO
	glGenVertexArrays(1, vao);
	glGenBuffers(2, vbo);

	glBindVertexArray(vao[0]); // bind VAO 0 as current object
	// Position data in attribute index 0, 3 floats per vertex
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // bind VBO for positions

	glBufferData(GL_ARRAY_BUFFER, numParticles * 3 * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);
	for (int i = 0; i < numParticles; i++) {
		glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * i, 3 * sizeof(GLfloat), glm::value_ptr(positions[i]));
	}
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);     // Enable attribute index 0

									  // Colours data in attribute 1, 3 floats per vertex
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // bind VBO for colours
	glBufferData(GL_ARRAY_BUFFER, numParticles * 3 * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);
	for (int i = 0; i < numParticles; i++) {
		glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * i, 3 * sizeof(GLfloat), glm::value_ptr(colours[i]));
	}
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);    // Enable attribute index 1

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void particleArray::draw(void)
{
	//glPointSize(100.0f);
	glPointSize(30);
	glBindVertexArray(vao[0]); // bind VAO 0 as current object
							   // particle data may have been updated - so need to resend to GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // bind VBO 0

	//glBufferData(GL_ARRAY_BUFFER, numParticles * 3 * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);
	for (int i = 0; i < numParticles; i++)
	{
		glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat)*i, 3 * sizeof(GLfloat), glm::value_ptr(positions[i]));
	}
	// Position data in attribute index 0, 3 floats per vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);     // Enable attribute index 0

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // bind VBO 0

	//glBufferData(GL_ARRAY_BUFFER, numParticles * 3 * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);
	for (int i = 0; i < numParticles; i++)
	{
		glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat)*i, 3 * sizeof(GLfloat), glm::value_ptr(colours[i]));
	}
	// Position data in attribute index 0, 3 floats per vertex
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);     // Enable attribute index 0
									  // Now draw the particles... as easy as this!
	glDrawArrays(GL_POINTS, 0, numParticles);
	glBindVertexArray(0);
}

void particleArray::update(void)
{
	
	n_time += 0.33f;
	d_time = (n_time - l_time);
	life_time -= d_time;
	c_time += d_time;

	for (int i = 1; i < numParticles; i++)
	{
		vel[i].y -= grav;
		if (vel[i].y <= 0)
			vel[i].y = 0;
		positions[i].y += (vel[i].y * d_time);
	}

	if (c_time >= 0.5f && blend)
	{
		for (int i = 0; i < numParticles; i++)
		{
			colours[i].r -= 0.025f;
			if (colours[i].r <= 0.2f)
			{
				if (positions[i].y >= 2.3f)
				{
					vel[i].x = vel2[i].x;
					vel[i].z = vel2[i].z;
				}
				colours[i].r = 0.2f;
			}

			colours[i].g -= 0.05f;
			if (colours[i].g <= 0.2f)
				colours[i].g = 0.2f;

			colours[i].b -= 0.2f;
			if (colours[i].b <= 0.2f)
				colours[i].b = 0.2f;

			c_time = 0;
		}
	}

	for (int i = 0; i < numParticles; i++)
	{
		if (positions[i].y >= 0.05f)
		{
			positions[i].x += (vel[i].x * d_time);
			positions[i].z += (vel[i].z * d_time);
		}
	}
	for (int i = 0; i < numParticles; i++)
	{
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_KP_0])
			positions[i].x += (vel3[i].x * d_time);
		
		if (keys[SDL_SCANCODE_RETURN])
			blend = true;
	}

	l_time = n_time;
}

particleArray::~particleArray()
{
	delete[] positions;
	delete[] colours;
	delete[] vel;
}
