//
//	qcpu c++ version
//

#include "Application.h"

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
	}

	return 0;
}