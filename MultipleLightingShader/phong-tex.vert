// phong-tex.vert
// Vertex shader for use with a Phong or other reflection model fragment shader
// Calculates and passes on V, L, N vectors for use in fragment shader, phong2.frag
#version 330

#define LIGHT_COUNT 3

uniform mat4 modelview;
uniform mat4 projection;
//uniform mat3 normalmatrix;
struct lightStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
};
in  vec3 in_Position;
in  vec3 in_Normal;


in vec2 in_TexCoord;

out VertexData
{
	vec3 ex_N;
	vec3 ex_V;
	vec3 ex_L[3];
	vec2 ex_TexCoord;

	vec4 position;
	
}vertex;

layout (std140) uniform Lights
{
	lightStruct lights[LIGHT_COUNT];

}; 

// multiply each vertex position by the MVP matrix
// and find V, L, N vectors for the fragment shader
void main(void) {

	// vertex into eye coordinates
	vec4 vertexPosition = modelview * vec4(in_Position,1.0);

	// Find V - in eye coordinates, eye is at (0,0,0)
	vertex.ex_V = normalize(-vertexPosition).xyz;

	// surface normal in eye coordinates
	// taking the rotation part of the modelview matrix to generate the normal matrix
	// (if scaling is includes, should use transpose inverse modelview matrix!)
	// this is somewhat wasteful in compute time and should really be part of the cpu program,
	// giving an additional uniform input

	mat3 normalmatrix = transpose(inverse(mat3(modelview)));
	vertex.ex_N = normalize(normalmatrix * in_Normal);

	// L - to light source from vertex
	vertex.ex_L[0] = lights[0].position.xyz - vertexPosition.xyz;
	vertex.ex_L[1] = lights[1].position.xyz - vertexPosition.xyz;
	vertex.ex_L[2] = lights[2].position.xyz - vertexPosition.xyz;

	vertex.ex_TexCoord = in_TexCoord;

    gl_Position = projection * vertexPosition;

}