#include "MainWindow.h"

int Main(int argc, char** argv)
{
	MainWindow window("Visual INI", argc, argv);
	MainWindow::Instance = &window;

	if (window.Create())
		return window.Run();

	return 0;
}