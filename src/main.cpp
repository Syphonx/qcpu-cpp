//
//	qcpu c++ version
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
			avengers.AssembleAndSave(argv[2]);
		}
	}

	return 0;
}