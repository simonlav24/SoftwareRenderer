
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

vec3 hsv2rgb(in vec3 hsv);
vec3 rgb2hsv(in vec3 rgb);

float gen_noise(in vec2 coordinates);
vec3 woodColor(in vec2 coordinates);

uniform mat4 lookAt;
uniform mat4 proj;
uniform mat4 worldModelMat;
uniform mat4 normalMat;

uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform	vec3 matEmissive;
uniform float matShininess;

uniform vec3 viewerPos;
uniform bool isTexturized;
uniform bool isEnvironment;
uniform bool isNormalMap;
uniform float environmentStrength;
uniform bool isNoiseTexture;

// point lights
uniform vec3 lightPositions[MAX_NUM_OF_LIGHTS];
uniform vec3 lightColors[MAX_NUM_OF_LIGHTS];
uniform int lightTypes[MAX_NUM_OF_LIGHTS];
uniform int lightCount;
uniform int isColorAnimating;
uniform float timeStep;

in vec4 posInCam;
in vec3 normalInCam;
in vec2 TexCoord;

in vec3 vPosition;
in vec3 vNormal;
in vec3 modelPosition;

in mat3 TBN;

out vec4 fcolor;
uniform sampler2D materialTexture;
uniform sampler2D environmentTexture;
uniform sampler2D normalMapTexture;
uniform sampler2D noiseTexture;

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
    totalColor += calculateSpecularLight(posInCam, vec4(normal, 0.0));
    totalColor += matEmissive;

    if(isColorAnimating == 1)
    {
        float hue = 78.0f * sqrt(modelPosition.x * modelPosition.x + modelPosition.z * modelPosition.z) + 360.0f * timeStep / TWOPI;
        totalColor *= hsv2rgb(vec3(hue, 1.0f, 1.0f));
    }
        
    if(isTexturized)
    {
        totalColor *= texture(materialTexture, TexCoord).xyz;
    }

    if (isNoiseTexture)
    {
        vec3 turbulanceColor = vec3(0.0f, 0.0f, 0.0f);
        float sizeMult = 0.125f;
        turbulanceColor += texture(noiseTexture, TexCoord * 1.0f * sizeMult).xyz * 0.5;
        turbulanceColor += texture(noiseTexture, TexCoord * 2.0f * sizeMult).xyz * 0.25;
        turbulanceColor += texture(noiseTexture, TexCoord * 4.0f * sizeMult).xyz * 0.1875;
        turbulanceColor += texture(noiseTexture, TexCoord * 8.0f * sizeMult).xyz * 0.0625;

        float xyPeriod = 80.0;
        float turbPower = 0.1;
        float turbSize = 32.0;
        
        float xval = (TexCoord.x - 512 / 2) / 512;
        float yval = (TexCoord.y - 512 / 2) / 512;
        
        float distValue = sqrt(xval * xval + yval * yval) + turbPower * turbulanceColor.x;
        float sinValue = 0.5f * abs(sin(2.0f * xyPeriod * distValue * PI));
        vec3 color = vec3(0.3125f + sinValue, 0.1171875f + sinValue, 0.1171875);

        totalColor *= color;
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
        diffusionAmount = lightColors[i] * matDiffuse * dotProd;
        totalDiffusionLight += diffusionAmount;
        
    }

    return totalDiffusionLight;
}

vec3 calculateSpecularLight(in vec4 position, in vec4 normal)
{
    vec3 pos = position.xyz;
    vec3 normalInCam = normal.xyz;

    vec3 totalSpecularLight = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < lightCount; i++)
    {
        vec3 specularAmount = vec3(0.0, 0.0, 0.0);
        vec3 dirFromLight;
        if(lightTypes[i] == LIGHT_AMBIENT)
            continue;
        if(lightTypes[i] == LIGHT_POINT)
            dirFromLight = -normalize((lookAt * vec4(lightPositions[i], 1.0)).xyz - pos);
        if(lightTypes[i] == LIGHT_PARALLEL)
            dirFromLight = normalize((lookAt * vec4(lightPositions[i], 0.0)).xyz);
        vec3 reflected = dirFromLight - 2.0 * dot(dirFromLight, normalInCam) * normalInCam;
        vec3 dirToViewer = (lookAt * vec4(viewerPos, 1.0)).xyz - pos;
        float dotProd = dot(normalize(reflected), normalize(dirToViewer));

        if (dotProd <= 0)
			continue;

		dotProd = pow(dotProd, matShininess);
        specularAmount = lightColors[i] * matSpecular * dotProd;
        totalSpecularLight += specularAmount;
    }
    return totalSpecularLight;
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

vec3 woodColor(in vec2 coordinates){
	vec2 pos = coordinates.yx * vec2(2, -7);
	float deg = gen_noise(pos) * 7.7;
	mat2 rotate = mat2(cos(deg), - sin(deg), sin(deg), cos(deg)) ;
	pos= rotate* pos;
	//smoothstep is just like activation function in ML
	float pattern = smoothstep(0.0, 1.0, abs(cos(pos.x * 5.0) - 1.0) * 0.8);
	return mix(vec3(0.5,0.2,0.1), vec3(0.8,0.5,0.4), pattern);
}

float gen_noise(in vec2 coordinates){
	return pow(sin(coordinates.x),2)-cos(coordinates.y);	
}