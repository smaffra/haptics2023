#pragma once

#include <GL/gl.h>
#include <mutex>
#include "tracking_event.hpp"

class UserData;

class TrackingWindow
{
protected:
	UserData& _user_data;
	GLuint _vbo_grid_vertices = 0;
	GLuint _vbo_grid_indices = 0;
	GLuint _vbo_grid_lines_indices = 0;
	bool _buffers_ready = false;
	GLfloat _board_width = 0.10;
	GLfloat _board_height = 0.10;
	TrackingEvent _event;
	std::mutex _mutex;

	int nbFrames = 0;
	double lastTime=0.0;

public:
	TrackingWindow(UserData&);
	virtual ~TrackingWindow();
	
	void init(int w, int h, double xmin, double xmax, double ymin, double ymax);
	void display();
	void idle();
	void render();
	void keyboard(unsigned char key, int x, int y);
	void keyboard_up(unsigned char key, int x, int y);
	void keyboard_special(int key, int x, int y);
	void keyboard_special_up(int key, int x, int y);

	void update(const TrackingEvent&);
};
