//
//	qcpu c++ version
//

#include "Application.h"

int main(int argc, char *argv[])
{
	int num_args = argc - 1;
	if (num_args <= 0)
	{
		std::cout << "Please provide a file" << std::endl;
	}
	else
	{
		Application app(argv[1]);
		if (num_args > 1)
		{
			app.ReadInput(argv[2]);
		}
		return app.Run();
	}

	return 0;
}