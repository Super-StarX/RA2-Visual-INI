#include "MainWindow.h"

int Main(int argc, char** argv)
{
	MainWindow window("Visual INI", argc, argv);

	if (window.Create())
		return window.Run();

	return 0;
}