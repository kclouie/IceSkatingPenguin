#version 330 core
uniform sampler2D Texture0;

in vec2 vTexCoord;
in vec3 vColor;

out vec4 Outcolor;

void main() {
  vec4 BaseColor = vec4(vColor.x, vColor.y, vColor.z, 1);
  vec4 texColor0 = texture(Texture0, vTexCoord);
 
  	Outcolor = vec4(texColor0.r, texColor0.g, texColor0.b, 1);
  //DEBUG:Outcolor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
}

