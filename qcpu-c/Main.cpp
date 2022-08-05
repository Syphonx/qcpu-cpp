//
//	qcpu-c - qcpu compiler
//

#include "Assembler.h"

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		std::cout << "Please provide a file" << std::endl;
	}
	else if (argc == 3)
	{
		Assembler avengers(argv[1]);
		avengers.AssembleAndSave(argv[2]);
	}

	return 0;
}