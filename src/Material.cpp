#include "Material.h"
#include "image/stb_image.h"
#include <iostream>
#include <glad/glad.h>

#include <vector>

GLuint LoadTexture(const std::string& path, bool single);

void Material::LoadLUT() {
	stbi_set_flip_vertically_on_load(false); // LUT dose not need to flip
    int lutW = 0, lutH = 0, lutComp = 0;

	// 2D LUT for E
    std::string path = "Textures/E_LUT_128x128.png";
    float* data = stbi_loadf(path.c_str(), &lutW, &lutH, &lutComp, 1);
    if (!data) {
        std::cerr << "Failed to load E_LUT: " << path << std::endl;
        return;
    }
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, lutW, lutH, 0, GL_RED, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    LUT_E = tex;
    hasLutE = true;
    std::cout << "LUT_E size=" << lutW << "x" << lutH << " ifmt=" << std::hex << lutComp << std::dec << "\n";

    // 1D LUT for Eavg
    path = "Textures/Eavg_LUT_256x1.png";
    lutW = 0;
    lutH = 0;
    lutComp = 0;
    data = stbi_loadf(path.c_str(), &lutW, &lutH, &lutComp, 1);
    if (!data) {
        std::cerr << "Failed to load EAVG_LUT: " << path << std::endl;
        return;
    }
    tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, lutW, lutH, 0, GL_RED, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    LUT_Eavg = tex;
    hasLutEavg = true;
    std::cout << "LUT_Eavg size=" << lutW << "x" << lutH << " ifmt=" << std::hex << lutComp << std::dec << "\n";
}


void Material::LoadFromDirectory(const std::string& dir, const std::string& base) {

	Reset();

    std::vector<std::string> extensions = { ".png", ".jpg", ".jpeg", ".tga", ".bmp" /*,".exr"*/ };

    auto tryLoad = [&](const std::string& suffix, GLuint& tex, bool& hasFlag) {
        if (hasFlag) return;

        for (const auto& ext : extensions) {
            std::string path = dir + "/" + base + "_" + suffix + ext;
            bool single = false; // for single channel 
            if (suffix == "roughness" || suffix == "rough" || suffix == "disp") {
                single = true;
            }
            tex = LoadTexture(path, single);
            if (tex != 0) {
				std::cout << "Loaded texture: " << path << std::endl;
                hasFlag = true;
                return;
            }
        }
        hasFlag = false;
        };

    tryLoad("albedo", albedoMap, hasAlbedoMap);
    tryLoad("basecolor", albedoMap, hasAlbedoMap);
    tryLoad("normal", normalMap, hasNormalMap);
    tryLoad("nor", normalMap, hasNormalMap);
    tryLoad("roughness", roughnessMap, hasRoughnessMap);
    tryLoad("rough", roughnessMap, hasRoughnessMap);
    tryLoad("metallic", metallicMap, hasMetallicMap);
    tryLoad("metal", metallicMap, hasMetallicMap);
    tryLoad("ao", aoMap, hasAOMap);
    tryLoad("disp", displacementMap, hasDisplacementMap);
}

GLuint LoadTexture(const std::string& path, bool single) {
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (!data) return 0;

    GLenum format = (ch == 3) ? GL_RGB : GL_RGBA;
	if (single) {
		format = GL_RED;
	}
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return texID;
}

void Material::Bind(Shader& shader) {
    shader.use();

    int texSlot = 0;

    auto bindMap = [&](const char* name, GLuint id, bool present) {
        if (present) {
            glActiveTexture(GL_TEXTURE0 + texSlot);
            glBindTexture(GL_TEXTURE_2D, id);
            shader.setInt(name, texSlot++);
        }
        };

    bindMap("albedoMap", albedoMap, hasAlbedoMap);
    bindMap("normalMap", normalMap, hasNormalMap);
    bindMap("metallicMap", metallicMap, hasMetallicMap);
    bindMap("roughnessMap", roughnessMap, hasRoughnessMap);
    bindMap("aoMap", aoMap, hasAOMap);
    bindMap("displacementMap", displacementMap, hasDisplacementMap);

	// Bind LUTs on slot 8 and 9
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, LUT_E);
    shader.setInt("LUT_E", 8);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, LUT_Eavg);
    shader.setInt("LUT_Eavg", 9);


    // for shader
    shader.setBool("hasAlbedoMap", hasAlbedoMap);
    shader.setBool("hasNormalMap", hasNormalMap);
    shader.setBool("hasMetallicMap", hasMetallicMap);
    shader.setBool("hasRoughnessMap", hasRoughnessMap);
    shader.setBool("hasAOMap", hasAOMap);
	shader.setBool("hasDisplacementMap", hasDisplacementMap);

    // set default value for shader
    shader.setVec3("defaultAlbedo", defaultAlbedo);
    shader.setFloat("defaultMetallic", defaultMetallic);
    shader.setFloat("defaultRoughness", defaultRoughness);
    shader.setFloat("defaultAO", defaultAO);

	shader.setFloat("displacementScale", displacementScale);

}

void Material::Reset() {
    hasAlbedoMap = false;
    hasNormalMap = false;
    hasMetallicMap = false;
    hasRoughnessMap = false;
    hasAOMap = false;
    hasDisplacementMap = false;
}

