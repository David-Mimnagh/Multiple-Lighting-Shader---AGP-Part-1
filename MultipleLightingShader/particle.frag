// Fragment Shader – file "minimal.frag"

#version 330

in  vec4 ex_Color;
out vec4 out_Color;
uniform sampler2D textureUnit0;

void main(void)
{
	out_Color = ex_Color * texture(textureUnit0, gl_PointCoord);
}