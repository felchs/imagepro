// Include standard headers
#include "render.h"

int main(void)
{
	using namespace imagepro;
	using namespace std;

	CRender* render = new CRender();
	render->Run();
	delete render;

	return 0;
}