
#version 150 
#define TWOPI 6.2831853071795864769252867665590
#define PI 3.1415926535897932384626433832795
in vec4 color;
in vec2 vTexture;
in vec2 vTextureCoordinates;

uniform bool isTexturized;
uniform bool isEnvironment;
uniform bool isNoiseTexture;
uniform float environmentStrength;

uniform int isColorAnimating;
uniform float timeStep;
vec3 hsv2rgb(in vec3 hsv);
vec3 rgb2hsv(in vec3 rgb);

out vec4 fcolor;
in vec3 totalColorOut;
in vec2 environmentCoords;

uniform sampler2D materialTexture;
uniform sampler2D environmentTexture;
uniform sampler2D noiseTexture;

void main()
{
	vec3 totalColor = totalColorOut;
	if (isTexturized)
	{
		totalColor = totalColor * texture(materialTexture, vTextureCoordinates).xyz;
	}

	if (isNoiseTexture)
    {
        vec3 turbulanceColor = vec3(0.0f, 0.0f, 0.0f);
        float sizeMult = 0.125f;
        turbulanceColor += texture(noiseTexture, vTextureCoordinates * 1.0f * sizeMult).xyz * 0.5;
        turbulanceColor += texture(noiseTexture, vTextureCoordinates * 2.0f * sizeMult).xyz * 0.25;
        turbulanceColor += texture(noiseTexture, vTextureCoordinates * 4.0f * sizeMult).xyz * 0.1875;
        turbulanceColor += texture(noiseTexture, vTextureCoordinates * 8.0f * sizeMult).xyz * 0.0625;

        float xyPeriod = 80.0;
        float turbPower = 0.1;
        float turbSize = 32.0;

        float xval = (vTextureCoordinates.x - 512 / 2) / 512;
        float yval = (vTextureCoordinates.y - 512 / 2) / 512;
        
        float distValue = sqrt(xval * xval + yval * yval) + turbPower * turbulanceColor.x;
        float sinValue = 0.5f * abs(sin(2.0f * xyPeriod * distValue * PI));
        vec3 color = vec3(0.3125f + sinValue, 0.1171875f + sinValue, 0.1171875);

        totalColor *= color;
    }

	if (isEnvironment)
	{
		vec3 environmentColor = texture(environmentTexture, environmentCoords).xyz;
		totalColor = environmentStrength * environmentColor + (1.0f - environmentStrength) * totalColor;
	}

	if(isColorAnimating == 2)
    {
        vec3 hsv = rgb2hsv(totalColor);
        hsv.x += 360 * timeStep / TWOPI;
        totalColor = hsv2rgb(hsv);
    }

	fcolor = vec4(totalColor, 1.0f);
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