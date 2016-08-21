// Phong fragment shader phong-tex.frag matched with phong-tex.vert
#version 330

// Some drivers require the following
precision highp float;

struct lightStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
};

struct materialStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};
#define LIGHT_COUNT 3

uniform float attQuadratic = 1.0f;
uniform float attLinear = 0.2f;
uniform float attConst = 0.1f;
out vec4 out_Color;
uniform materialStruct material;

in VertexData
{
	vec3 ex_N;
	vec3 ex_V;
	vec3 ex_L[3];
	vec2 ex_TexCoord;
	float ex_D[3];

	vec4 position;
	
}vertex;

layout (std140) uniform Lights
{
	lightStruct lights[LIGHT_COUNT];

}; 

vec4 CalculateColour(lightStruct light, vec3 LightVector)
{
	float dist = length(LightVector);
	float attenuation = 1.0f / (attConst + attLinear * dist + attQuadratic * dist * dist);

		// Ambient intensity
	vec4 ambientI = light.ambient * material.ambient;

	// Diffuse intensity
	vec4 diffuseI = light.diffuse * material.diffuse;
	diffuseI = diffuseI * (max(dot(normalize(vertex.ex_N),normalize(LightVector)),0) * light.diffuse);

	// Specular intensity
	// Calculate R - reflection of light
	vec3 R = normalize(reflect(normalize(-LightVector),normalize(vertex.ex_N)));

	vec4 specularI = light.specular * material.specular;
	specularI = specularI * (pow(max(dot(R,vertex.ex_V),0), material.shininess)* light.specular);

	//Attenuation does not affect transparency
	vec4 litColour = vec4((specularI + diffuseI).rgb * attenuation, 1.0);
	vec4 amb = min(ambientI,vec4(1.0f));
	
	return litColour + amb;//Here attenuation does not affectambient
}
void main(void) 
{
	out_Color = CalculateColour(lights[0], vertex.ex_L[0]);
	out_Color += CalculateColour(lights[1], vertex.ex_L[1]);
	out_Color += CalculateColour(lights[2], vertex.ex_L[2]);
}