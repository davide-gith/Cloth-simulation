#include <glm/vec3.hpp>
#include "Globals.h"
#include "camera.h"

Camera cam;  // definizione globale (definizione e inizializzazione)


//Definizione sun
Light sun = {
	//glm::vec3(-5.0f, 7.0f, 6.0f), // Position
	glm::vec3(0.0f, 0.0f, -1.0f), // Parallel light direction
	glm::vec3(1.0f, 1.0f, 1.0f)   // Color
};