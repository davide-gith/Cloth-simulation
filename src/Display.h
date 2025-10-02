#ifndef DISPLAY_H
#define DISPLAY_H

#include <iostream>
#include <vector>
#include "Program.h"
#include "shader/shader_m.h"
#include "image/stb_image.h"
#include "Vertex.h"
#include "ClothData.h"
#include "ClothInstance.h"
#include "Globals.h"
#include "Material.h"
#include "Ground.h"
#include "Sphere.h"
#include "Box.h"
#include "Capsule.h"
#include "SweptSphere.h"
#include "SweptSphereTri.h"


struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

struct ClothRender {
    const ClothInstance* clothInstance;
    const ClothData* cloth;
    int nodeCount;

    glm::vec3* vboPos;
    glm::vec2* vboTex;
    glm::vec3* vboNor;
    glm::vec3* vboTan;

    GLuint vaoID;
    GLuint vboIDs[4]; // pos, tex, norm, tan

    Material material;
    Shader shader;

    std::string dir;
    std::string basename;

    ClothRender(ClothInstance* clothI, std::string dir, std::string basename)
        : shader("Shaders/PBR.vs", "Shaders/PBR.fs"),
          dir(dir),
          basename(basename){
        this->clothInstance = clothI;
        this->cloth = clothI->getData();
        this->material.LoadFromDirectory(dir, basename);

        nodeCount = (int)cloth->faces.size();
        if (nodeCount <= 0) {
            std::cerr << "ERROR::ClothRender : No nodes." << std::endl;
            exit(-1);
        }

        vboPos = new glm::vec3[nodeCount];
        vboTex = new glm::vec2[nodeCount];
        vboNor = new glm::vec3[nodeCount];
        vboTan = new glm::vec3[nodeCount];

        for (int i = 0; i < nodeCount; ++i) {
            Node* n = cloth->faces[i];
            vboPos[i] = glm::vec3(n->position.x, n->position.y, n->position.z);
            vboTex[i] = glm::vec2(n->texCoord.x, n->texCoord.y);
            vboNor[i] = glm::vec3(n->normal.x, n->normal.y, n->normal.z);
            vboTan[i] = glm::vec3(n->tangent.x, n->tangent.y, n->tangent.z);
        }

        glGenVertexArrays(1, &vaoID);
        glGenBuffers(4, vboIDs);

        glBindVertexArray(vaoID);

        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glBufferData(GL_ARRAY_BUFFER, nodeCount * sizeof(glm::vec3), vboPos, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glBufferData(GL_ARRAY_BUFFER, nodeCount * sizeof(glm::vec2), vboTex, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
        glBufferData(GL_ARRAY_BUFFER, nodeCount * sizeof(glm::vec3), vboNor, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
        glBufferData(GL_ARRAY_BUFFER, nodeCount * sizeof(glm::vec3), vboTan, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(3);

        glBindVertexArray(0);
    }

    void flush() {
        for (int i = 0; i < nodeCount; ++i) {
            Node* n = cloth->faces[i];
            vboPos[i] = glm::vec3(n->position.x, n->position.y, n->position.z);
            vboNor[i] = glm::vec3(n->normal.x, n->normal.y, n->normal.z);
        }

        shader.use();
        glBindVertexArray(vaoID);

        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, nodeCount * sizeof(glm::vec3), vboPos);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, nodeCount * sizeof(glm::vec2), vboTex);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, nodeCount * sizeof(glm::vec3), vboNor);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, nodeCount * sizeof(glm::vec3), vboTan);

        // vertex shader
        material.Bind(shader);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(cloth->clothPos.x, cloth->clothPos.y, cloth->clothPos.z));
        cam.uniViewMatrix = glm::lookAt(cam.Position, cam.Position + cam.Front, cam.Up);
        glm::mat4 view = cam.uniViewMatrix;
        glm::mat4 proj = cam.uniProjMatrix;

        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", proj);
        shader.setVec3("camPos", cam.Position);

        shader.setVec3("lightDir", sun.dir);
        shader.setVec3("lightColor", sun.color);

        glDisable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        //Draw
        switch (this->clothInstance->drawMode) {
            case ClothInstance::DRAW_NODES:
                glDrawArrays(GL_POINTS, 0, nodeCount);
                break;
            case ClothInstance::DRAW_LINES:
                glDrawArrays(GL_LINES, 0, nodeCount);
                break;
            default:
                glDrawArrays(GL_TRIANGLES, 0, nodeCount);
                break;
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);


    }

    void destroy() {
        delete[] vboPos;
        delete[] vboTex;
        delete[] vboNor;
        delete[] vboTan;
        glDeleteVertexArrays(1, &vaoID);
        glDeleteBuffers(4, vboIDs);
    }
};

struct SpringRender
{
    std::vector<Spring*> springs;
    int springCount; // Number of nodes in springs
    
    glm::vec4 uniSpringColor;
    
    glm::vec3 *vboPos; // Position
    glm::vec3 *vboNor; // Normal

    GLuint programID;
    GLuint vaoID;
    GLuint vboIDs[2];
    
    GLint aPtrPos;
    GLint aPtrNor;
    
    // Render any spring set, color and modelVector
    void init(std::vector<Spring*> s, glm::vec4 c, glm::vec3 modelVec)
    {
        springs = s;
        springCount = (int)(springs.size());
        if (springCount <= 0) {
            std::cout << "ERROR::SpringRender : No node exists." << std::endl;
            exit(-1);
        }
        
        uniSpringColor = c;
        
        vboPos = new glm::vec3[springCount*2];
        vboNor = new glm::vec3[springCount*2];
        for (int i = 0; i < springCount; i ++) {
            Node* node1 = springs[i]->node1;
            Node* node2 = springs[i]->node2;
            vboPos[i*2] = glm::vec3(node1->position.x, node1->position.y, node1->position.z);
            vboPos[i*2+1] = glm::vec3(node2->position.x, node2->position.y, node2->position.z);
            vboNor[i*2] = glm::vec3(node1->normal.x, node1->normal.y, node1->normal.z);
            vboNor[i*2+1] = glm::vec3(node2->normal.x, node2->normal.y, node2->normal.z);
        }
        
        //Build render program
        Program program("Shaders/SpringVS.glsl", "Shaders/SpringFS.glsl");
        programID = program.ID;
        //std::cout << "Spring Program ID: " << programID << std::endl;

        // Generate ID of VAO and VBOs
        glGenVertexArrays(1, &vaoID);
        glGenBuffers(2, vboIDs);
        
        // Attribute pointers of VAO
        aPtrPos = 0;
        aPtrNor = 1;
        // Bind VAO
        glBindVertexArray(vaoID);
        
        // Position buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glVertexAttribPointer(aPtrPos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, springCount*2*sizeof(glm::vec3), vboPos, GL_DYNAMIC_DRAW);
        // Normal buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glVertexAttribPointer(aPtrNor, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, springCount*2*sizeof(glm::vec3), vboNor, GL_DYNAMIC_DRAW);
        
        // Enable it's attribute pointers since they were set well
        glEnableVertexAttribArray(aPtrPos);
        glEnableVertexAttribArray(aPtrNor);
        
        //Set uniform
        glUseProgram(programID); // Active shader before set uniform
        // Set color
        glUniform4fv(glGetUniformLocation(programID, "uniSpringColor"), 1, &uniSpringColor[0]);
        
        //Projection matrix : The frustum that camera observes
        // Since projection matrix rarely changes, set it outside the rendering loop for only onec time
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniProjMatrix"), 1, GL_FALSE, &cam.uniProjMatrix[0][0]);
        
        //Model Matrix : Put rigid into the world
        glm::mat4 uniModelMatrix = glm::mat4(1.0f);
        uniModelMatrix = glm::translate(uniModelMatrix, modelVec);
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniModelMatrix"), 1, GL_FALSE, &uniModelMatrix[0][0]);
        
        //Light
        glUniform3fv(glGetUniformLocation(programID, "uniLightDir"), 1, &(sun.dir[0]));
        glUniform3fv(glGetUniformLocation(programID, "uniLightColor"), 1, &(sun.color[0]));

        // Cleanup
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbined VBO
        glBindVertexArray(0); // Unbined VAO
    }
    
    void destroy()
    {
        delete [] vboPos;
        delete [] vboNor;
        
        if (vaoID)
        {
            glDeleteVertexArrays(1, &vaoID);
            glDeleteBuffers(2, vboIDs);
            vaoID = 0;
        }
        if (programID)
        {
            glDeleteProgram(programID);
            programID = 0;
        }
    }
    
    void flush() // Rigid does not move, thus do not update vertexes' data
    {
        // Update all the positions of nodes
        for (int i = 0; i < springCount; i ++) {
            Node* node1 = springs[i]->node1;
            Node* node2 = springs[i]->node2;
            vboPos[i*2] = glm::vec3(node1->position.x, node1->position.y, node1->position.z);
            vboPos[i*2+1] = glm::vec3(node2->position.x, node2->position.y, node2->position.z);
            vboNor[i*2] = glm::vec3(node1->normal.x, node1->normal.y, node1->normal.z);
            vboNor[i*2+1] = glm::vec3(node2->normal.x, node2->normal.y, node2->normal.z);
        }
        
        glUseProgram(programID);
        
        glBindVertexArray(vaoID);
        
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, springCount*2*sizeof(glm::vec3), vboPos);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, springCount*2*sizeof(glm::vec3), vboNor);
        
        //View Matrix : The camera
        cam.uniViewMatrix = glm::lookAt(cam.Position, cam.Position + cam.Front, cam.Up);
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniViewMatrix"), 1, GL_FALSE, &cam.uniViewMatrix[0][0]);
        
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        //Draw
        glDrawArrays(GL_LINES, 0, springCount*2);
        
        // End flushing
        glDisable(GL_BLEND);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};

struct ClothSpringRender
{
    ClothInstance *cloth;
    glm::vec4 defaultColor;
    SpringRender render;
    
    ClothSpringRender(ClothInstance* c)
    {
        cloth = c;
        defaultColor = glm::vec4(0.25, 0.05, 0.0, 1.0);
        render.init(cloth->getData()->springs, defaultColor, glm::vec3(cloth->getData()->clothPos.x, cloth->getData()->clothPos.y, cloth->getData()->clothPos.z));
    }
    
    void flush() { render.flush(); }
};

struct RigidRender // Single color & Lighting
{
    std::vector<Vertex*> faces;
    int vertexCount; // Number of nodes in faces
    
    glm::vec4 uniRigidColor;
    
    glm::vec3 *vboPos; // Position
    glm::vec3 *vboNor; // Normal

    GLuint programID;
    GLuint vaoID;
    GLuint vboIDs[2];
    
    GLint aPtrPos;
    GLint aPtrNor;
    
    // Render any rigid body only with it's faces, color and modelVector
    void init(std::vector<Vertex*> f, glm::vec4 c, glm::vec3 modelVec)
    {
        faces = f;
        vertexCount = (int)(faces.size());
        if (vertexCount <= 0) {
            std::cout << "ERROR::RigidRender : No vertex exists." << std::endl;
            exit(-1);
        }
        
        uniRigidColor = c;
        
        vboPos = new glm::vec3[vertexCount];
        vboNor = new glm::vec3[vertexCount];
        for (int i = 0; i < vertexCount; i ++) {
            Vertex* v = faces[i];
            vboPos[i] = glm::vec3(v->position.x, v->position.y, v->position.z);
            vboNor[i] = glm::vec3(v->normal.x, v->normal.y, v->normal.z);
        }
        
        //Build render program
        Program program("Shaders/RigidVS.glsl", "Shaders/RigidFS.glsl");
        programID = program.ID;
        //std::cout << "Rigid Program ID: " << programID << std::endl;

        // Generate ID of VAO and VBOs
        glGenVertexArrays(1, &vaoID);
        glGenBuffers(2, vboIDs);
        
        // Attribute pointers of VAO
        aPtrPos = 0;
        aPtrNor = 1;
        // Bind VAO
        glBindVertexArray(vaoID);
        
        // Position buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glVertexAttribPointer(aPtrPos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(glm::vec3), vboPos, GL_DYNAMIC_DRAW);
        // Normal buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glVertexAttribPointer(aPtrNor, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(glm::vec3), vboNor, GL_DYNAMIC_DRAW);
        
        // Enable it's attribute pointers since they were set well
        glEnableVertexAttribArray(aPtrPos);
        glEnableVertexAttribArray(aPtrNor);
        
        //Set uniform
        glUseProgram(programID); // Active shader before set uniform
        // Set color
        glUniform4fv(glGetUniformLocation(programID, "uniRigidColor"), 1, &uniRigidColor[0]);
        
        //Projection matrix : The frustum that camera observes
        // Since projection matrix rarely changes, set it outside the rendering loop for only onec time
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniProjMatrix"), 1, GL_FALSE, &cam.uniProjMatrix[0][0]);
        
        //Model Matrix : Put rigid into the world
        glm::mat4 uniModelMatrix = glm::mat4(1.0f);
        uniModelMatrix = glm::translate(uniModelMatrix, modelVec);
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniModelMatrix"), 1, GL_FALSE, &uniModelMatrix[0][0]);
        
        //Light
        glUniform3fv(glGetUniformLocation(programID, "uniLightDir"), 1, &(sun.dir[0]));
        glUniform3fv(glGetUniformLocation(programID, "uniLightColor"), 1, &(sun.color[0]));

        // Cleanup
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbined VBO
        glBindVertexArray(0); // Unbined VAO
    }
    
    void destroy()
    {
        delete [] vboPos;
        delete [] vboNor;
        
        if (vaoID)
        {
            glDeleteVertexArrays(1, &vaoID);
            glDeleteBuffers(2, vboIDs);
            vaoID = 0;
        }
        if (programID)
        {
            glDeleteProgram(programID);
            programID = 0;
        }
    }
    
    void flush() // Rigid does not move, thus do not update vertexes' data
    {
        for (int i = 0; i < vertexCount; i++) {
            Vertex* v = faces[i];
            vboPos[i] = glm::vec3(v->position.x, v->position.y, v->position.z);
            vboNor[i] = glm::vec3(v->normal.x, v->normal.y, v->normal.z);
        }

        glUseProgram(programID);

        glBindVertexArray(vaoID);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(glm::vec3), vboPos);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(glm::vec3), vboNor);

        //View Matrix : The camera
        cam.uniViewMatrix = glm::lookAt(cam.Position, cam.Position + cam.Front, cam.Up);
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniViewMatrix"), 1, GL_FALSE, &cam.uniViewMatrix[0][0]);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Draw
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);

        // End flushing
        glDisable(GL_BLEND);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};

struct GroundRender
{
    Ground* ground;
    RigidRender render;

    GroundRender(Ground* g)
    {
        ground = g;
        render.init(ground->faces, ground->color, glm::vec3(ground->position.x, ground->position.y, ground->position.z));
    }

    void flush() { render.flush(); }
};

struct SphereRender
{
    Sphere* sphere;
    RigidRender render;

    SphereRender(Sphere* c)
    {
        sphere = c;
        render.init(sphere->faces, sphere->color, glm::vec3(sphere->center.x, sphere->center.y, sphere->center.z));
        //render.init(sphere->faces, sphere->color, glm::vec3(0.0, 0.0, 0.0));
    }

    void flush() { render.flush(); }
};

struct BoxRender
{
    Box* box;
    RigidRender render;

    BoxRender(Box* b)
    {
        box = b;
        //render.init(box->faces, box->color, glm::vec3(box->center.x, box->center.y, box->center.z));
        render.init(box->faces, box->color, glm::vec3(0.0, 0.0, 0.0));
    }

    void flush() { render.flush(); }
};

struct CapsuleRender
{
    Capsule* capsule;
    RigidRender render;

    CapsuleRender(Capsule* c)
    {
        capsule = c;
        //render.init(capsule->faces, capsule->color, glm::vec3(capsule->center.x, capsule->center.y, capsule->center.z));
        render.init(capsule->faces, capsule->color, glm::vec3(0.0, 0.0, 0.0));
    }

    void flush() { render.flush(); }
};

struct SweptSphereRender
{
    SweptSphere* sweptSphere;
    RigidRender render;

    SweptSphereRender(SweptSphere* ss)
    {
        sweptSphere = ss;
        render.init(sweptSphere->faces, sweptSphere->color, glm::vec3(0.0, 0.0, 0.0));
    }

    void flush() { render.flush(); }
};

struct SweptSphereTriRender
{
    SweptSphereTri* sweptSphereTri;
    RigidRender render;

    SweptSphereTriRender(SweptSphereTri* ssT)
    {
        sweptSphereTri = ssT;
        render.init(sweptSphereTri->faces, sweptSphereTri->color, glm::vec3(0.0, 0.0, 0.0));
    }

    void flush() { render.flush(); }
};
#endif





// --------------------------- OLD --------------------------------//
// struct ClothRender // Texture & Lighting
//{
//	const ClothInstance* clothInstance; // Cloth instance
//    const ClothData* cloth;
//    int nodeCount; // Number of all nodes in faces
//    
//    glm::vec3 *vboPos; // Position
//    glm::vec2 *vboTex; // Texture
//    glm::vec3 *vboNor; // Normal
//
//    GLuint programID;
//    GLuint vaoID;
//    GLuint vboIDs[3];
//    GLuint texID;
//    
//    GLint aPtrPos;
//    GLint aPtrTex;
//    GLint aPtrNor;
//
//    
//    ClothRender(ClothInstance* clothI)
//    {
//		this->clothInstance = clothI;
//        nodeCount = (int)(clothI->getData()->faces.size());
//        if (nodeCount <= 0) {
//            std::cout << "ERROR::ClothRender : No node exists." << std::endl;
//            exit(-1);
//        }
//        
//        this->cloth = clothI->getData();
//        
//        vboPos = new glm::vec3[nodeCount];
//        vboTex = new glm::vec2[nodeCount];
//        vboNor = new glm::vec3[nodeCount];
//        for (int i = 0; i < nodeCount; i ++) {
//            Node* n = cloth->faces[i];
//            vboPos[i] = glm::vec3(n->position.x, n->position.y, n->position.z);
//            vboTex[i] = glm::vec2(n->texCoord.x, n->texCoord.y); // Texture coord will only be set here
//            vboNor[i] = glm::vec3(n->normal.x, n->normal.y, n->normal.z);
//        }
//        
//        //Build render program
//        Program program("Shaders/ClothVS.glsl", "Shaders/ClothFS.glsl");
//        programID = program.ID;
//        //std::cout << "Cloth Program ID: " << programID << std::endl;
//
//        // Generate ID of VAO and VBOs
//        glGenVertexArrays(1, &vaoID);
//        glGenBuffers(3, vboIDs);
//        
//        // Attribute pointers of VAO
//        aPtrPos = 0;
//        aPtrTex = 1;
//        aPtrNor = 2;
//        // Bind VAO
//        glBindVertexArray(vaoID);
//        
//        // Position buffer
//        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
//        glVertexAttribPointer(aPtrPos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
//        glBufferData(GL_ARRAY_BUFFER, nodeCount*sizeof(glm::vec3), vboPos, GL_DYNAMIC_DRAW);
//        // Texture buffer
//        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
//        glVertexAttribPointer(aPtrTex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
//        glBufferData(GL_ARRAY_BUFFER, nodeCount*sizeof(glm::vec2), vboTex, GL_DYNAMIC_DRAW);
//        // Normal buffer
//        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
//        glVertexAttribPointer(aPtrNor, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
//        glBufferData(GL_ARRAY_BUFFER, nodeCount*sizeof(glm::vec3), vboNor, GL_DYNAMIC_DRAW);
//        
//        // Enable it's attribute pointers since they were set well
//        glEnableVertexAttribArray(aPtrPos);
//        glEnableVertexAttribArray(aPtrTex);
//        glEnableVertexAttribArray(aPtrNor);
//        
//        //Load texture
//        // Assign texture ID and gengeration
//        glGenTextures(1, &texID);
//        glBindTexture(GL_TEXTURE_2D, texID);
//        // Set the texture wrapping parameters (for 2D tex: S, T)
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        // Set texture filtering parameters (Minify, Magnify)
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        //Load image and configure texture
//        stbi_set_flip_vertically_on_load(true);
//
//
//
//        int texW, texH, colorChannels; // After loading the image, stb_image will fill them
//        //unsigned char *data = stbi_load("Textures/light_blue.png", &texW, &texH, &colorChannels, 0);
//        //unsigned char* data = stbi_load("Textures/fabric_leather_02_diff_4k.jpg", &texW, &texH, &colorChannels, 0);
//        unsigned char* data = stbi_load("Textures/fabric_pattern_07_col_1_4k.png", &texW, &texH, &colorChannels, 0);
//        if (data) {
//            glTexImage2D(GL_TEXTURE_2D, 0, (colorChannels == 4) ? GL_RGBA : GL_RGB, texW, texH, 0, (colorChannels == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
//            //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, texW, texH, 0, GL_RGB, GL_FLOAT, data);
//
//            // Automatically generate all the required mipmaps for the currently bound texture.
//            glGenerateMipmap(GL_TEXTURE_2D);
//        } else {
//            std::cout << "Failed to load texture" << std::endl;
//        }
//        // Always free image memory
//        stbi_image_free(data);
//
//        
//        //Set uniform
//        glUseProgram(programID); // Active shader before set uniform
//        // Set texture sampler
//        glUniform1i(glGetUniformLocation(programID, "uniTex"), 0);
//        
//        //Projection matrix : The frustum that camera observes
//        // Since projection matrix rarely changes, set it outside the rendering loop for only onec time
//        glUniformMatrix4fv(glGetUniformLocation(programID, "uniProjMatrix"), 1, GL_FALSE, &cam.uniProjMatrix[0][0]);
//        
//        //Model Matrix : Put cloth into the world
//        glm::mat4 uniModelMatrix = glm::mat4(1.0f);
//        uniModelMatrix = glm::translate(uniModelMatrix, glm::vec3(cloth->clothPos.x, cloth->clothPos.y, cloth->clothPos.z));
//        glUniformMatrix4fv(glGetUniformLocation(programID, "uniModelMatrix"), 1, GL_FALSE, &uniModelMatrix[0][0]);
//        
//        //Light
//        glUniform3fv(glGetUniformLocation(programID, "uniLightPos"), 1, &(sun.pos[0]));
//        glUniform3fv(glGetUniformLocation(programID, "uniLightColor"), 1, &(sun.color[0]));
//
//        // Cleanup
//        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbined VBO
//        glBindVertexArray(0); // Unbined VAO
//    }
//    
//
//    void destroy()
//    {
//        delete [] vboPos;
//        delete [] vboTex;
//        delete [] vboNor;
//        
//        if (vaoID)
//        {
//            glDeleteVertexArrays(1, &vaoID);
//            glDeleteBuffers(3, vboIDs);
//            vaoID = 0;
//        }
//        if (programID)
//        {
//            glDeleteProgram(programID);
//            programID = 0;
//        }
//    }
//    
//    void flush()
//    {
//        // Update all the positions of nodes
//        for (int i = 0; i < nodeCount; i ++) { // Tex coordinate dose not change
//            Node* n = cloth->faces[i];
//            vboPos[i] = glm::vec3(n->position.x, n->position.y, n->position.z);
//            vboNor[i] = glm::vec3(n->normal.x, n->normal.y, n->normal.z);
//        }
//        
//        glUseProgram(programID);
//        
//        glBindVertexArray(vaoID);
//        
//        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
//        glBufferSubData(GL_ARRAY_BUFFER, 0, nodeCount*sizeof(glm::vec3), vboPos);
//        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
//        glBufferSubData(GL_ARRAY_BUFFER, 0, nodeCount* sizeof(glm::vec2), vboTex);
//        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
//        glBufferSubData(GL_ARRAY_BUFFER, 0, nodeCount* sizeof(glm::vec3), vboNor);
//        
//        //Bind texture
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, texID);
//        
//        //View Matrix : The camera
//        cam.uniViewMatrix = glm::lookAt(cam.Position, cam.Position + cam.Front, cam.Up);
//        glUniformMatrix4fv(glGetUniformLocation(programID, "uniViewMatrix"), 1, GL_FALSE, &cam.uniViewMatrix[0][0]);
//        
//        glEnable(GL_BLEND);
//        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        
//        //Draw
//        switch (this->clothInstance->drawMode) {
//            case ClothInstance::DRAW_NODES:
//                glDrawArrays(GL_POINTS, 0, nodeCount);
//                break;
//            case ClothInstance::DRAW_LINES:
//                glDrawArrays(GL_LINES, 0, nodeCount);
//                break;
//            default:
//                glDrawArrays(GL_TRIANGLES, 0, nodeCount);
//                break;
//        }
//
//        // End flushing
//        glDisable(GL_BLEND);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        glBindVertexArray(0);
//        glUseProgram(0);
//    }
//};
