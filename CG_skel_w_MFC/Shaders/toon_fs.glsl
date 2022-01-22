
#version 150 
#define MAX_NUM_OF_LIGHTS 6
#define LIGHT_POINT 0
#define LIGHT_PARALLEL 1
#define LIGHT_AMBIENT 2

#define PI 3.1415926535897932384626433832795
#define TWOPI 6.2831853071795864769252867665590

vec3 calculateAmbientLight();
vec3 calculateDiffusionLight(in vec4 position, in vec4 normal);
vec3 calculateSpecularLight(in vec4 position, in vec4 normal);
vec3 calculateEnvironment(in vec4 position, in vec4 normal);
float quantizeBrightness(float b);

vec3 hsv2rgb(in vec3 hsv);
vec3 rgb2hsv(in vec3 rgb);

uniform mat4 lookAt;
uniform mat4 proj;
uniform mat4 worldModelMat;
uniform mat4 normalMat;

uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform	vec3 matEmissive;
uniform float matShininess;

uniform int quantizationNum;
uniform int isColorAnimating;
uniform float timeStep;

uniform vec3 viewerPos;
uniform bool isTexturized;
uniform bool isEnvironment;
uniform bool isNormalMap;
uniform float environmentStrength;

// point lights
uniform vec3 lightPositions[MAX_NUM_OF_LIGHTS];
uniform vec3 lightColors[MAX_NUM_OF_LIGHTS];
uniform int lightTypes[MAX_NUM_OF_LIGHTS];
uniform int lightCount;

in vec4 posInCam;
in vec3 normalInCam;
in vec2 TexCoord;

in vec3 vPosition;
in vec3 vNormal;

in mat3 TBN;

out vec4 fcolor;
uniform sampler2D materialTexture;
uniform sampler2D environmentTexture;
uniform sampler2D normalMapTexture;

void main()
{
    vec3 normal = normalInCam;

    if(isNormalMap)
	{
        normal = TBN * normalize(texture2D(normalMapTexture, TexCoord).xyz * 2.0f - 1.0f);
	}

	vec3 totalColor = vec3(0.0, 0.0, 0.0);
    totalColor += calculateAmbientLight();
    totalColor += calculateDiffusionLight(posInCam, vec4(normal, 0.0));
    totalColor += matEmissive;
    
    if(isColorAnimating == 1)
    {
        vec4 modelPosition = transpose(lookAt) * posInCam;
        float hue = 78.0f * sqrt(modelPosition.x * modelPosition.x + modelPosition.z * modelPosition.z) + 360.0f * timeStep / TWOPI;
        totalColor *= hsv2rgb(vec3(hue, 1.0f, 1.0f));
    }

    if(isTexturized)
    {
        totalColor *= texture(materialTexture, TexCoord).xyz;
    }

    if(isColorAnimating == 2)
    {
        vec3 hsv = rgb2hsv(totalColor);
        hsv.x += 360 * timeStep / TWOPI;
        totalColor = hsv2rgb(hsv);
    }

    if(isEnvironment)
    {
        vec3 enviroColor = calculateEnvironment(posInCam, vec4(normal, 1.0f));
        totalColor = environmentStrength * enviroColor + (1.0f - environmentStrength) * totalColor;
    }

    fcolor = vec4(totalColor, 1.0f);
}

vec3 calculateAmbientLight()
{
    vec3 totalAmbientLight = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < lightCount; i++)
    {
        totalAmbientLight += lightColors[i] * matAmbient;
    }
    return totalAmbientLight;
}

vec3 calculateDiffusionLight(in vec4 position, in vec4 normal)
{
    vec3 pos = position.xyz;
    vec3 normalInCam = normal.xyz;

    vec3 totalDiffusionLight = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < lightCount; i++)
    {
        vec3 diffusionAmount = vec3(0.0, 0.0, 0.0);
        vec3 dirFromLight;
        if(lightTypes[i] == LIGHT_AMBIENT)
            continue;
        if(lightTypes[i] == LIGHT_POINT)
            dirFromLight = normalize((lookAt * vec4(lightPositions[i], 1.0)).xyz - pos);
        if(lightTypes[i] == LIGHT_PARALLEL)
            dirFromLight = -normalize((lookAt * vec4(lightPositions[i], 0.0)).xyz);
        float dotProd = max(0.0, dot(normalize(normalInCam), normalize(dirFromLight)));

        // dot product is the brightness. quantize this
        dotProd = quantizeBrightness(dotProd);

        diffusionAmount = lightColors[i] * matDiffuse * dotProd;
        totalDiffusionLight += diffusionAmount;
        
    }
    return totalDiffusionLight;
}

vec3 calculateEnvironment(in vec4 position, in vec4 normal)
{
    vec3 dirToViewer = normalize((lookAt * vec4(viewerPos, 1.0)).xyz - position.xyz);
    vec3 reflected = reflect(dirToViewer, normal.xyz);
    reflected = -normalize(reflected); 

    reflected = (transpose(lookAt) * vec4(reflected, 1.0)).xyz;

    float r = sqrt(reflected.x * reflected.x + reflected.y * reflected.y + reflected.z * reflected.z);
    float theta = (atan(reflected.z, reflected.x) + PI) / TWOPI;
    float phi = 1.0f - acos(reflected.y / r) / PI;
    vec2 sphericCoord = vec2(theta, phi);

    return texture(environmentTexture, sphericCoord).xyz;
}

float quantizeBrightness(float b)
{
    float q = quantizationNum;
    return floor(q * ((1.0f + 1.0f/q) * b)) / q;
}


vec3 rgb2hsv(in vec3 rgb)
{
    float fR = rgb.x;
    float fG = rgb.y;
    float fB = rgb.z;

    float fH, fS, fV;

    float fCMax = max(max(fR, fG), fB);
    float fCMin = min(min(fR, fG), fB);
    float fDelta = fCMax - fCMin;
  
    if(fDelta > 0) {
      if(fCMax == fR) {
        fH = 60 * (mod(((fG - fB) / fDelta), 6));
      } else if(fCMax == fG) {
        fH = 60 * (((fB - fR) / fDelta) + 2);
      } else if(fCMax == fB) {
        fH = 60 * (((fR - fG) / fDelta) + 4);
      }
      
      if(fCMax > 0) {
        fS = fDelta / fCMax;
      } else {
        fS = 0;
      }
      
      fV = fCMax;
    } else {
      fH = 0;
      fS = 0;
      fV = fCMax;
    }
    
    if(fH < 0) {
      fH = 360 + fH;
    }
    return vec3(fH, fS, fV);
}

vec3 hsv2rgb(in vec3 hsv)
{
	float s = hsv.y;
	float v = hsv.z;
	float C = s * v;
    float h = mod(hsv.x, 360.0f);
	float X = C * (1 - abs(mod(h / 60.0, 2.0) - 1.0));
	float m = v - C;
	float r, g, b;
	if (h >= 0 && h < 60) {
		r = C, g = X, b = 0;
	}
	else if (h >= 60 && h < 120) {
		r = X, g = C, b = 0;
	}
	else if (h >= 120 && h < 180) {
		r = 0, g = C, b = X;
	}
	else if (h >= 180 && h < 240) {
		r = 0, g = X, b = C;
	}
	else if (h >= 240 && h < 300) {
		r = X, g = 0, b = C;
	}
	else {
		r = C, g = 0, b = X;
	}
	float R = (r + m);
	float G = (g + m);
	float B = (b + m);
	return vec3(R, G, B);
}