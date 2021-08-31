#version 130

in vec4 vPosition;
in vec4 vColor;
out vec4 color;

uniform mat4 ModelView;

void main() 
{
	gl_Position = ModelView * vPosition;
	color = vColor;	
} 
