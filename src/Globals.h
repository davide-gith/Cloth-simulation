#ifndef GLOBALS_H
#define GLOBALS_H

#include "camera.h"
#include <glm/vec3.hpp>

extern Camera cam;  // dichiarazione globale (solo dichiarazione)

struct Light
{
	glm::vec3 dir;
	glm::vec3 color;
};

extern Light sun; // dichiarazione globale (solo dichiarazione)

#endif