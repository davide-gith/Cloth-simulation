#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aNormal;
layout (location = 1) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

out vec3 FragPos;
out vec2 TexCoord;
out vec3 WorldNormal;
out mat3 TBN;
out vec3 camPosition;

// Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camPos;
uniform bool hasNormalMap;

uniform sampler2D displacementMap;
uniform bool hasDisplacementMap;
uniform float displacementScale;

void main()
{
    camPosition = camPos;

    TexCoord = aTexCoord;

    // N
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * aNormal);

    // Displacement
    vec3 displacedPos = aPos;
    if (hasDisplacementMap) {
        float height = texture(displacementMap, aTexCoord).r; // height ∈ [0,1]
        displacedPos += aNormal * (height - 0.5) * displacementScale;
    }

    // New Position
    vec4 worldPos = model * vec4(displacedPos, 1.0);
    FragPos = worldPos.xyz;
    WorldNormal = N;

    // TBN
    if (hasNormalMap) {
        vec3 T = normalize(mat3(model) * aTangent);
        vec3 B = normalize(cross(N, T));
        TBN = mat3(T, B, N);
    } else {
        TBN = mat3(1.0); // fallback
    }

    gl_Position = projection * view * worldPos;
}
