#pragma once
#ifndef INTERFACE_H
#define INTERFACE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "src/Display.h"
#include "src/ClothInstance.h"
#include "src/IClothSimulator.h"
#include "src/SweptSphereTriCollider.h"
#include <random>

class Interface
{
public:
    GLFWwindow* window;
    ClothInstance* clothInstance;
    ClothRender* clothRender;
    ClothSpringRender* clothSpringRender;
    ClothInstance** clothInstancePtr;
    ClothRender** clothRenderPtr;
    ClothSpringRender** clothSpringRenderPtr;
    int& running;
    Camera& cam;

	Interface(GLFWwindow* window, ClothInstance** clothInstance, ClothRender** clothRender, ClothSpringRender** clothSpringRender, int& running, Camera& cam) 
        : clothInstancePtr(clothInstance),
          clothRenderPtr(clothRender),
          clothSpringRenderPtr(clothSpringRender), 
          running(running),
          cam(cam){
		this->window = window;
        this->clothInstance = *clothInstance;
        this->clothRender = *clothRender;
        this->clothSpringRender = *clothSpringRender;
		this->running = running;
		this->cam = cam;
		init();
	}


    void init() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext(NULL);
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void newFrame() {
        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawFPS();
        drawCamPosition();
        ImGui::NewLine();
        drawRunningControl();
        drawSelectModel();
		drawMethodControl();
		drawRestartButton();
		drawClothRenderMode();
        if (clothRender != nullptr && clothRender->material.hasDisplacementMap) {
            drawDisplacementController();
        }
		drawMaterialSelector();
		drawAddCollider();
        
        ImGui::End();
    }

    void render() {
        // ImGui render
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    int frameCount = 0;
    int frame = 0;
    double lastTime = glfwGetTime();

    void calculateFPS() {
        double currentTime = glfwGetTime();
        frameCount++;

        if (currentTime - lastTime >= 1.0) {
            frame = frameCount;
            frameCount = 0;
            lastTime = currentTime;
        }
    }

private:
    int selectedModelButton = 1;
	std::string model = "grid"; // Default model
	std::string method = "SymplecticEuler"; // Default method

    void drawFPS() {
        calculateFPS();

        ImGui::Begin("GUI");
        ImGui::Text("FPS:%d", frame);
    }

    void drawSelectModel() {
        ImGui::Text("Model");

        // Button Grid
        {
            bool selected = (selectedModelButton == 1);
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));

            if (ImGui::Button("Grid")) {
                selectedModelButton = 1;
				model = "grid";
            }

            if (selected)
                ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        // Button T-shirt
        {
            bool selected = (selectedModelButton == 2);
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));

            if (ImGui::Button("T-shirt")) {
                selectedModelButton = 2;
                model = "tshirt";
            }

            if (selected)
                ImGui::PopStyleColor();
        }
    }

    void drawRunningControl() {
		ImGui::Text("Running Control");
        if (running == 0) {
            if (ImGui::Button("Start")) {
                running = 1;
            }
        }
		if (running == 1) {
			if (ImGui::Button("Stop")) {
				running = 0;
			}
		}
    }
    
	int selectedMethodButton = 0;
    void drawMethodControl() {
        ImGui::Text("Method");

        // Button SymplecticEuler
        {
            bool selected = (selectedMethodButton == 0);
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));

            if (ImGui::Button("SymplecticEuler")) {
                selectedMethodButton = 0;
				method = "SymplecticEuler";
            }

            if (selected)
                ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        // Button ExplicitEuler
        {
            bool selected = (selectedMethodButton == 1);
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));

            if (ImGui::Button("ExplicitEuler")) {
                selectedMethodButton = 1;
				method = "ExplicitEuler";
            }

            if (selected)
                ImGui::PopStyleColor();
        }
        ImGui::SameLine();

        // Button PBD
        {
            bool selected = (selectedMethodButton == 2);
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));

            if (ImGui::Button("PBD")) {
                selectedMethodButton = 2;
				method = "PBD";
            }

            if (selected)
                ImGui::PopStyleColor();
        }
        ImGui::SameLine();

        // Button XPBD
        {
            bool selected = (selectedMethodButton == 3);
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));

            if (ImGui::Button("XPBD")) {
                selectedMethodButton = 3;
				method = "XPBD";
            }

            if (selected)
                ImGui::PopStyleColor();
        }
        ImGui::SameLine();

        // Button PD
        {
            bool selected = (selectedMethodButton == 4);
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));

            if (ImGui::Button("PD")) {
                selectedMethodButton = 4;
                method = "ProjectiveDynamics";
            }
            
            if (selected)
                ImGui::PopStyleColor();
        }
        ImGui::SameLine();
        

        // Button Newton
        {
            bool selected = (selectedMethodButton == 5);
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));

            if (ImGui::Button("Newton")) {
                selectedMethodButton = 5;
                method = "ImplicitNewton";
            }

            if (selected)
                ImGui::PopStyleColor();
        }
        ImGui::SameLine();

    }

	void drawRestartButton() {
        ImGui::NewLine();
        ImGui::Text("Restart for changing methods and models");
		if (ImGui::Button("Restart Simulation")) {
			restartSimulation();
		}
	}

	void restartSimulation() {
        delete *clothInstancePtr;
        delete *clothRenderPtr;
        delete *clothSpringRenderPtr;

        *clothInstancePtr = ClothInstance::create(model, method);
        *clothRenderPtr = new ClothRender(*clothInstancePtr, dir, basename);
        *clothSpringRenderPtr = new ClothSpringRender(*clothInstancePtr);

        clothInstance = *clothInstancePtr;
        clothRender = *clothRenderPtr;
        clothSpringRender = *clothSpringRenderPtr;

        running = 0;
	}

    void drawClothRenderMode() {
		ImGui::Text("Cloth Render Mode");
		if (ImGui::Button("Nodes")) {
			(*clothInstancePtr)->drawMode = ClothInstance::DRAW_NODES;
		}
		ImGui::SameLine();
		if (ImGui::Button("Lines")) {
			(*clothInstancePtr)->drawMode = ClothInstance::DRAW_LINES;
		}
		ImGui::SameLine();
		if (ImGui::Button("Faces")) {
			(*clothInstancePtr)->drawMode = ClothInstance::DRAW_FACES;
		}
    }

    // Camera Position
    void drawCamPosition() {
        ImGui::Text("Camera Postion: %f, %f, %f", cam.Position.x, cam.Position.y, cam.Position.z);
    }

    // Displacement Slider
    void drawDisplacementController(){
		float displacementScale = clothRender->material.displacementScale;
        ImGui::SliderFloat("Displacement Scale", &displacementScale, 0.0f, 2.0f);
		clothRender->material.displacementScale = displacementScale;
    }

    // Materials list
    std::vector<std::pair<std::string, std::string>> materialList = {
        { "Textures/Materials/Woolen", "RedWhite" },
        { "Textures/Materials/Woolen", "carpet" },
        { "Textures/Materials/leather", "leather" },
    };

    // Default material
	std::string dir = "Textures/Materials/Woolen";
	std::string basename = "RedWhite";

	void drawMaterialSelector() {
        static int selectedMaterialIndex = 0;

        if (ImGui::BeginCombo("Material", materialList[selectedMaterialIndex].second.c_str())) {
            for (int i = 0; i < materialList.size(); ++i) {
                bool isSelected = (selectedMaterialIndex == i);
                if (ImGui::Selectable(materialList[i].second.c_str(), isSelected)) {
                    selectedMaterialIndex = i;

                    // change material
                    clothRender->material.LoadFromDirectory(
                        materialList[i].first,
                        materialList[i].second
                    );

					dir = materialList[i].first;
					basename = materialList[i].second;

                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
	}

    void drawAddCollider() {
        ImGui::Text("Add Collider");

        if (ImGui::Button("Sphere")) {
			clothInstance->addCollider(new SphereCollider);
        }

        ImGui::SameLine();

        if (ImGui::Button("Box")) {
            clothInstance->addCollider(new AABBCollider);
        }

        ImGui::SameLine();

        if (ImGui::Button("Capsule")) {
			//clothInstance->addCollider(new CapsuleCollider(randomPosition(), randomPosition()));
			clothInstance->addCollider(new CapsuleCollider());
        }

        ImGui::SameLine();

        if (ImGui::Button("SweptSphere")) {
            clothInstance->addCollider(new SweptSphereCollider);
        }

        ImGui::SameLine();

        if (ImGui::Button("SweptSphereTri")) {
            clothInstance->addCollider(new SweptSphereTriCollider);
        }

        ImGui::SameLine();

        if (ImGui::Button("SphereMesh")) {
            clothInstance->addCollider(new SphereMeshesCollider("Mesh/female_neutral_SM.txt"));
        }
     
    }

    Vec3 randomPosition() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0.0f, 10.0f);
        return Vec3(dis(gen), dis(gen), dis(gen));
    }
};

#endif


