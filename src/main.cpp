#include <iostream>
#include "Renderer.h"

int main() {
	Renderer rd(1920, 1080, 20, "F:/VisualStudioCode/RGSRenderer/scene/scene04.xml");
	rd.Run();
	return 0;
}