#include "utils.hpp" 
#include <GL/glut.h>
#include <cstdio>
#include <sstream>
#include <stdexcept>

const char* special_key_str(int key)
{
	switch (key)
	{
	case GLUT_KEY_F1: return "F1";
	case GLUT_KEY_F2: return "F2";
	case GLUT_KEY_F3: return "F3";
	case GLUT_KEY_F4: return "F4";
	case GLUT_KEY_F5: return "F5";
	case GLUT_KEY_F6: return "F6";
	case GLUT_KEY_F7: return "F7";
	case GLUT_KEY_F8: return "F8";
	case GLUT_KEY_F9: return "F9";
	case GLUT_KEY_F10: return "F10";
	case GLUT_KEY_F11: return "F11";
	case GLUT_KEY_F12: return "F12";
	case GLUT_KEY_LEFT: return "LEFT";
	case GLUT_KEY_UP: return "UP";
	case GLUT_KEY_RIGHT: return "RIGHT";
	case GLUT_KEY_DOWN: return "DOWN";
	case GLUT_KEY_PAGE_UP: return "PAGE_UP";
	case GLUT_KEY_PAGE_DOWN: return "PAGE_DOWN";
	case GLUT_KEY_HOME: return "HOME";
	case GLUT_KEY_END: return "END";
	case GLUT_KEY_INSERT: return "INSERT";
	}
	return "UNDEFINED";
}

std::string default_output_filename(void)
{
	std::string base_filename = "tracking_data.avro";

	for (int i = 0; i < 100; i++)
	{
		if (i == 0)
		{
			FILE* f = fopen(base_filename.c_str(), "rb");
			if(f == NULL)
				return base_filename;
		}
		else
		{
			std::ostringstream sstr;
			sstr << base_filename << " (" << i << ").avro";
			FILE* f = fopen(sstr.str().c_str(), "rb");
			if (f == NULL)
				return sstr.str();
		}
	}

	throw std::runtime_error("Too many copies of the base output file were found!");
}
