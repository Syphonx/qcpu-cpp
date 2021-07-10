//
// qcpu c++ version
//

#include "application.h"
#include "assembler.h"

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		std::cout << "Please provide a file" << std::endl;
	}
	else
	{
		if (argc == 2)
		{
			Application app(argv[1]);
			return app.Run();
		}
		else
		{
			Assembler avengers(argv[1]);
			const std::vector<uint8_t>& text = avengers.Assemble();
			std::ofstream file(argv[2], std::ios::out | std::ios::binary);
			if (!text.empty())
			{
				file.write(reinterpret_cast<const char*>(text.data()), text.size());
			}
		}
	}

	return 0;
}