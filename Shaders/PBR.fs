#version 330 core

in vec3 FragPos;
in vec2 TexCoord;
in vec3 WorldNormal;
in mat3 TBN;
in vec3 camPosition;

out vec4 FragColor;

// ======= Texture maps =======
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// ======= Map presence flags =======
uniform bool hasAlbedoMap;
uniform bool hasNormalMap;
uniform bool hasMetallicMap;
uniform bool hasRoughnessMap;
uniform bool hasAOMap;

// ======= Defaults =======
uniform vec3  defaultAlbedo    = vec3(0.5);
uniform float defaultRoughness = 0.5;
uniform float defaultMetallic  = 0.0;
uniform float defaultAO        = 1.0;

// ======= Directional light =======
uniform vec3 lightDir;       // Parallel light
uniform vec3 lightColor;     
uniform float lightIntensity = 1.0;

// ======= Kulla–Conty LUTs =======
// 2D E(mu, alpha) : x = mu, y = alpha(=roughness^2)
uniform sampler2D LUT_E;
// 1D Eavg(alpha)  : x = alpha, y = 0.5
uniform sampler2D LUT_Eavg;

const float PI = 3.14159265359;

// ----------------- Helpers -----------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float D_GGX(float NdotH, float alpha)
{
    float a2   = alpha * alpha;
    float ndh2 = NdotH * NdotH;
    float denom = ndh2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float G1_SchlickGGX(float NdotX, float k)
{
    return NdotX / (NdotX * (1.0 - k) + k);
}

float G_Smith_SchlickGGX(float NdotV, float NdotL, float roughness)
{
    float a = roughness + 1.0;
    float k = (a * a) * 0.125; // (alpha')^2 / 8 
    float Gv = G1_SchlickGGX(NdotV, k);
    float Gl = G1_SchlickGGX(NdotL, k);
    return Gv * Gl;
}

// ---- LUT lookups                               
float lookupE(float mu, float alpha)              // 2D：x=mu, y=alpha
{
    mu    = clamp(mu,    0.0, 1.0);
    alpha = clamp(alpha, 0.0, 1.0);
    return texture(LUT_E, vec2(mu, alpha)).r;
}

float lookupEavg(float alpha)                     // 1D：x=alpha, y=0.5
{
    alpha = clamp(alpha, 0.0, 1.0);
    return texture(LUT_Eavg, vec2(alpha, 0.5)).r;
}

// F_avg = 1/21 + 20/21 * F0
vec3 fresnelAverage(vec3 F0)
{
    return vec3(1.0/21.0) + (20.0/21.0) * F0;
}

// Kulla–Conty Color term C = (Favg * Eavg)/(1 - Favg*(1 - Eavg))
vec3 kcColorTerm(vec3 Favg, float Eavg)
{
    vec3 denom = vec3(1.0) - Favg * (1.0 - Eavg);
    denom = max(denom, vec3(1e-4));
    return (Favg * Eavg) / denom;
}

void main()
{
    // ---- Material ----
    vec3  albedo    = hasAlbedoMap    ? pow(texture(albedoMap, TexCoord).rgb, vec3(2.2)) : defaultAlbedo; // sRGB->Linear
    float roughness = hasRoughnessMap ? texture(roughnessMap, TexCoord).r                : defaultRoughness;
    float metallic  = hasMetallicMap  ? texture(metallicMap,  TexCoord).r                : defaultMetallic;
    float ao        = hasAOMap        ? texture(aoMap,        TexCoord).r                : defaultAO;

    roughness = clamp(roughness, 0.04, 1.0);
    metallic  = clamp(metallic,  0.0,  1.0);
    ao        = clamp(ao,        0.0,  1.0);

    // ---- Normal (TBN) ----
    vec3 N = normalize(WorldNormal);
    if (hasNormalMap) {
        vec3 tN = texture(normalMap, TexCoord).rgb * 2.0 - 1.0; // tangent space
        N = normalize(TBN * tN);
    }

    vec3 V = normalize(camPosition - FragPos);
    if (dot(N, V) < 0.0) N = -N;

    // ---- Light / Half ----
    vec3 L = normalize(-lightDir);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    vec3 radiance = lightColor * lightIntensity;

    // ---- F0 ----
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // ---- Single scattering (GGX) ----
    float alpha = roughness * roughness;          // alpha = r^2
    float D = D_GGX(NdotH, alpha);
    float G = G_Smith_SchlickGGX(NdotV, NdotL, roughness);
    vec3  F = fresnelSchlick(HdotV, F0);

    vec3 spec_single = (F * (G * D)) / max(4.0 * NdotV * NdotL, 1e-4);

    // ---- Multi-scattering (Kulla–Conty) ----
    float Eo   = lookupE(NdotV, alpha);          // E(mu_o, alpha)
    float Ei   = lookupE(NdotL, alpha);          // E(mu_i, alpha)
    float Eavg = lookupEavg(alpha);              // E_avg(alpha)

    Eo   = clamp(Eo,   0.0, 0.999);
    Ei   = clamp(Ei,   0.0, 0.999);
    Eavg = clamp(Eavg, 0.0, 0.999);

    //f_ms_unc = ((1-Eo)*(1-Ei)) / (pi*(1-Eavg))
    float f_ms_unc = ((1.0 - Eo) * (1.0 - Ei)) / max(PI * (1.0 - Eavg), 1e-3);

    // Color term (per-channel)
    vec3 Favg = fresnelAverage(F0);
    vec3 Cms  = kcColorTerm(Favg, Eavg);

    vec3 spec_ms  = Cms * f_ms_unc;            
    vec3 specular = spec_single + spec_ms;  

    // ---- Diffuse ----
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = (albedo / PI) * kD;

    vec3 Lo = (diffuse + specular) * radiance * NdotL;

    // AO
    vec3 color = Lo * ao;

    // Gamma
    color = pow(max(color, 0.0), vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
