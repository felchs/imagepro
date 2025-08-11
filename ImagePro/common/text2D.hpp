#ifndef TEXT2D_HPP
#define TEXT2D_HPP

namespace imagepro
{
	void initText2D(const char* texturePath);
	void printText2D(const char* text, int x, int y, int size);
	void cleanupText2D();
}

#endif