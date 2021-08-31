#ifndef __ANGEL_H__
#define __ANGEL_H__

#include <cmath>
#include <iostream>
#include <cstdio>

//  Define M_PI.

#ifndef M_PI
#  define M_PI  3.14159
#endif

#ifdef __APPLE__  // include Mac OS X verions of headers
#  include <OpenGL/gl3.h>
#  include <GL/glut.h>
#else // non-Mac OS X operating systems
#  include <GL/glew.h>
//#  include <GL/glut.h>
#  include <GL/freeglut.h>
#  include <GL/freeglut_ext.h>
#endif  // __APPLE__

// Define a helpful macro for handling offsets into buffer objects
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))


namespace Angel 
{
	GLuint InitShader(const char* vertexShaderFile,
		const char* fragmentShaderFile);
	const GLfloat  DivideByZeroTolerance = GLfloat(1.0e-07);
	const GLfloat  DegreesToRadians = M_PI / 180.0;
}  

#include "vec.h"
#include "mat.h"
#include "CheckError.h"

#define Print(x)  do { std::cerr << #x " = " << (x) << std::endl; } while(0)

using namespace Angel;

#endif 