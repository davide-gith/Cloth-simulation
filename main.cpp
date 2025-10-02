#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>

//Shader
#include "shader/shader_m.h"

//Load image
#include "image/stb_image.h"

//Math function (glm)
#include <glm/glm.hpp>

#include "src/Vectors.h"
#include "src/camera.h"
#include "src/Display.h"

#include "src/ClothInstance.h"
#include "src/IClothSimulator.h"
#include "src/SphereCollider.h"
#include "src/CapsuleCollider.h"
#include "src/AABBCollider.h"
#include "src/SweptSphereCollider.h"
#include "src/SphereMeshesCollider.h"
#include "src/SweptSphereTri.h"
#include "src/Interface.h"

// -------------------------------- Functions and Callback functions --------------------------------
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


// -------------------------------- General settings --------------------------------
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 800;


GLFWwindow* window; //Window
//Vec3 bgColor = Vec3(50.0 / 255, 50.0 / 255, 60.0 / 255); //Background color
Vec3 bgColor = Vec3(1, 1, 1);
int running = 0; //Execution flow
bool forc = false;



// -------------------------------- Wind --------------------------------
int windBlowing = 0;
int windForceScale = 15;
Vec3 windStartPos;
Vec3 windDir;
Vec3 wind;


// -------------------------------- Cloth --------------------------------
ClothInstance* cloth = nullptr;
ClothRender* clothRender = nullptr;
ClothSpringRender* clothSpringRender = nullptr;


// -------------------------------- Timing --------------------------------
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int frameCount = 0;


// -------------------------------- Mouse parameters --------------------------------
float lastX = 0;
float lastY = 0;
bool leftButton = false;
bool firstMouse = true;


int main()
{
	// -------------------------------- glfw: initialize and configure --------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	//Prepare for rendering -> Initialize GLFW
	window = glfwCreateWindow(WIDTH, HEIGHT, "Cloth Simulation Master", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Set the context of this window as the main context of current thread
	glfwMakeContextCurrent(window);

	// Initialize Glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Register callback functions that should be registered after creating window and before initializing render loop
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);

	//Flip texture
	stbi_set_flip_vertically_on_load(true);//Flip texture
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glPointSize(3);

	// Cloth and renders
	cloth = ClothInstance::create("grid", "SymplecticEuler");
	clothRender = new ClothRender(cloth, "Textures/Materials/Woolen", "RedWhite");
	clothSpringRender = new ClothSpringRender(cloth);

	// -------------------------------- Interface -------------------------------- **
	Interface interface(window, &cloth, &clothRender, &clothSpringRender, running, cam);

	float start_time = glfwGetTime();
	// -------------------------------- Render loop --------------------------------
	while (!glfwWindowShouldClose(window))
	{
		//Frame
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		++frameCount;

		interface.newFrame();

		//Check for events
		processInput(window);

		//Set background color (R, G, B, A)
		glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// -------------------------------- Simulation & Rendering --------------------------------
		if (running) {
			cloth->update();
		}

		cloth->computeNormal();

		// Display
		if (cloth->drawMode == ClothInstance::DRAW_LINES) {
			clothSpringRender->flush();
		}
		else {
			clothRender->flush();
		}

		if (cloth->getColliders().size() > 0) {
			for (auto* collider : cloth->getColliders()) {
				collider->render();
			}
		}

		interface.render();


		/*if ((glfwGetTime() - start_time) > 10)
			glfwSetWindowShouldClose(window, true);*/


			// -------------------------------- glfw --------------------------------
			//Swap buffers and poll IO events (keys pressed/released, mouse, etc.)
		glfwSwapBuffers(window);
		glfwPollEvents(); //Update the status of window
	}

	// Calculate FPS after the loop ends
	float elapsedTime = glfwGetTime() - start_time;
	float fps = frameCount / elapsedTime;
	printf("\nElapsed time: %f\n", elapsedTime);
	printf("Final FPS: %f\n", fps);

	// -------------------------------- glfw --------------------------------
	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}





// -------------------------------- Window --------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


// -------------------------------- Mouse buttons --------------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Start wind
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && running) {
		windBlowing = 1;
		//Set start point of wind direction
		windStartPos.setZeroVec();
		glfwGetCursorPos(window, &windStartPos.x, &windStartPos.y);
		windStartPos.y = -windStartPos.y; //Reverse y since the screen local in the fourth quadrant
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE && running) // End wind
	{
		windBlowing = 0;
		windDir.setZeroVec();
	}

	//Press left button
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		leftButton = true;
	//Release left button
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		leftButton = false;
}


// -------------------------------- Cursor position --------------------------------
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
	//Detect mouse movements -> important for camera zoom
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	//Compute the offset
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	//Enable camera rotation when the left mouse buttone was pressed
	if (leftButton == true)
		cam.ProcessMouseMovement(xoffset, yoffset);

}


// -------------------------------- Camera zoom in/out --------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (yoffset > 0)
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			cam.ProcessKeyboard(FORWARD, (deltaTime * 7.0f));
		else
			cam.ProcessKeyboard(FORWARD, (deltaTime * 2.5f));
	}
	else
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			cam.ProcessKeyboard(BACKWARD, (deltaTime * 7.0f));
		else
			cam.ProcessKeyboard(BACKWARD, (deltaTime * 2.5f));
	}
}


// -------------------------------- Keyboard input --------------------------------
void processInput(GLFWwindow* window)
{
	//Keyboard control -> If key did not get pressed it will return GLFW_RELEASE
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);


	//Set draw mode
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		cloth->drawMode = ClothInstance::DRAW_NODES;
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		cloth->drawMode = ClothInstance::DRAW_LINES;
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		cloth->drawMode = ClothInstance::DRAW_FACES;


	//Camera control : [W] [S] [A] [D] [Q] [E]
	//Uses time to calculate velocity
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cam.ProcessKeyboard(UP, (deltaTime * 2.5f));
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cam.ProcessKeyboard(DOWN, (deltaTime * 2.5f));
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cam.ProcessKeyboard(LEFT, (deltaTime * 2.5f));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cam.ProcessKeyboard(RIGHT, (deltaTime * 2.5f));
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			cam.ProcessKeyboard(FORWARD, (deltaTime * 7.0f));
		else
			cam.ProcessKeyboard(FORWARD, (deltaTime * 2.5f));
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			cam.ProcessKeyboard(BACKWARD, (deltaTime * 7.0f));
		else
			cam.ProcessKeyboard(BACKWARD, (deltaTime * 2.5f));


	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && running) {
		cloth->restart();
	}

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && running)
		cloth->unpin();

	//if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && running) {
	//    running = 0; //Pause simulation
	//    std::string shape;
	//    std::string method;
	//    std::cout << "Enter shape (type: grid, tshirt): ";
	//    std::cin >> shape;
	//    std::cout << "Enter method (type: SymplecticEuler, ExplicitEuler, PBD, XPBD): ";
	//    std::cin >> method;

	//    delete cloth;
	//    delete clothRender;
	//    delete clothSpringRender;

	//    cloth = ClothInstance::create(shape, method);
	//    clothRender = new ClothRender(cloth);
	//    clothSpringRender = new ClothSpringRender(cloth);
	//    running = 1;
	//}

	// Pause simulation 
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		running = 0;
		printf("Paused.\n");
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		running = 1;
		printf("Running..\n");
	}
}


