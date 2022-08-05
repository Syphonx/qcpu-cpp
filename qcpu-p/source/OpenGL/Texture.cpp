//
//	Texture
//

#include "OpenGL/Texture.h"
#include "Constants.h"

#include <glad/glad.h>
#include <vector>

Texture::Texture()
	: m_Id(-1)
{

}

void Texture::Create()
{
	glGenTextures(1, &m_Id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Id);

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	std::vector<GLubyte> pixels;
	int pixelIndex = 0;
	pixels.clear();
	pixels.resize(TEXTURE_WIDTH * TEXTURE_HEIGHT * 4);

	for (size_t i = 0; i < TEXTURE_WIDTH * TEXTURE_HEIGHT * 4; i++)
	{
		pixels[i] = rand() % 255;
	}

	glTexImage2D(
		GL_TEXTURE_2D,			// target
		0,						// mip
		GL_RGBA, 				// format
		TEXTURE_WIDTH, 			// width
		TEXTURE_HEIGHT, 		// heigh
		0, 						// border
		GL_BGRA, 				// format
		GL_UNSIGNED_INT_8_8_8_8_REV, 		// type
		pixels.data()			// pixels
	);

	glBindTexture(GL_TEXTURE_2D, 0); // unbind
}

