#include "MainWindow.h"

int Main(int argc, char** argv)
{
	MainWindow exampe("Blueprints", argc, argv);

	if (exampe.Create())
		return exampe.Run();

	return 0;
}