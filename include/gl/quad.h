//
//	Quad
//

#include <stdint.h>

class Shader;

class Quad
{
public:
									Quad();

	void							Create(const Shader& shader);

	uint32_t						vao;
	uint32_t						vbo;
	uint32_t						ebo;
};