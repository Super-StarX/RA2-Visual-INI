#include "MainWindow.h"
#include "version.h"

int Main(int argc, char** argv)
{
	MainWindow::SetIcon(LoadIcon(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDI_ICON1)
	));

	MainWindow window("Visual INI", argc, argv);
	MainWindow::Instance = &window;

	if (window.Create())
		return window.Run();

	return 0;
}