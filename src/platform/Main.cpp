//enet has to be included before other stuff, otherwise you will get linking errors
//don't forget to also link to it in the cmake (you have to uncomment the add_subdirectory line and also
//  add enet in the target_link_libraries list at the end
//#include <enet/enet.h>

#include <iostream>
#include <Window.h>

int main(int argc, char* argv[])
{
	//Window creation
	Window window;
	if (argc > 1)
	{
		//Path to file to directly open
		std::string filename = argv[1];
		window.CreateMainWindow(filename);
	}
	else
	{
		window.CreateMainWindow();
	}

	return 0;
}