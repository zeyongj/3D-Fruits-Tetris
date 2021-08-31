#version 130

in  vec4 color;
out vec4  fColor;

void main() 
{ 
		if(color.a == 0.0) discard;
    	fColor = color;
} 
