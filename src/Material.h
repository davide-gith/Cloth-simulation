#pragma once
#include <glad/glad.h>
#include <string>
#include <map>
#include <shader/shader_m.h>

class Material {
public:
    GLuint albedoMap = 0;
    GLuint normalMap = 0;
    GLuint metallicMap = 0;
    GLuint roughnessMap = 0;
    GLuint aoMap = 0;
	GLuint displacementMap = 0;

    // Kulla Conty precomputed map
    GLuint LUT_E = 0;
    GLuint LUT_Eavg = 0;

    bool hasAlbedoMap = false;
    bool hasNormalMap = false;
    bool hasMetallicMap = false;
    bool hasRoughnessMap = false;
    bool hasAOMap = false;
	bool hasDisplacementMap = false;

    bool hasLutE = false;
    bool hasLutEavg = false;
    
	float displacementScale = 0.0f;

    glm::vec3 defaultAlbedo = glm::vec3(0.5f);
    float defaultMetallic = 0.0f;
    float defaultRoughness = 0.5f;
    float defaultAO = 1.0f;

    void LoadFromDirectory(const std::string& directory, const std::string& basename);
    void LoadLUT();
    void Bind(Shader& shader);
    void Reset();
};
