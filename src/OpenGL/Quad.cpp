//
//	Quad
//

#include "OpenGL/Quad.h"
#include "OpenGL/Shader.h"

#include <glad/glad.h>

const static GLfloat s_Vertices[] = 
{
	-1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
	 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
	 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
	-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
};

const static GLuint s_Elements[] = 
{
	0, 1, 3, // First triangle
	1, 2, 3  // Second triangle
};

Quad::Quad()
{

}

void Quad::Create(const Shader& shader)
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(s_Vertices), s_Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_Elements), s_Elements, GL_STATIC_DRAW);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(shader.GetProgram(), "aPos");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

	GLint colAttrib = glGetAttribLocation(shader.GetProgram(), "aColor");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	GLint texAttrib = glGetAttribLocation(shader.GetProgram(), "aTexCoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
}

