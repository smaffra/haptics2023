#include<GL/glew.h>
#include<GL/glut.h>
#include<GL/freeglut.h>

#include "tracking_window.hpp"
#include "user_data.hpp"

static const GLfloat grid_vertices_position[18] = {
  -1.0,  1.0,
   0.0,  1.0,
   1.0,  1.0,
  -1.0,  0.0,
   0.0,  0.0,
   1.0,  0.0,
  -1.0, -1.0,
   0.0, -1.0,
   1.0, -1.0
};

GLuint grid_indices[9] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8
};

GLuint grid_lines_indices[12] = {
	0, 6, 6, 8, 8, 2, 2, 0, 1, 7, 3, 5
};


TrackingWindow* getWindow(void)
{
	return static_cast<TrackingWindow*> (glutGetWindowData());
}

void tracking_window_display() 
{
	getWindow()->display();
}

void tracking_window_keyboard(unsigned char key, int x, int y)
{
	getWindow()->keyboard(key, x, y);
}

void tracking_window_keyboard_up(unsigned char key, int x, int y)
{
	getWindow()->keyboard_up(key, x, y);
}

void tracking_window_keyboard_special(int key, int x, int y)
{
	getWindow()->keyboard_special(key, x, y);
}

void tracking_window_keyboard_special_up(int key, int x, int y)
{
	getWindow()->keyboard_special_up(key, x, y);
}

void tracking_window_idle(void)
{
	getWindow()->idle();
}

TrackingWindow::TrackingWindow(UserData& p) : _user_data(p)
{
}

TrackingWindow::~TrackingWindow()
{
}

void TrackingWindow::init(int w, int h, double xmin, double xmax, double ymin, double ymax)
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(w, h);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Palm position tracking");
	glutSetWindowData((void*) this);
	glutDisplayFunc(tracking_window_display);
	glutIdleFunc(tracking_window_idle);
	glutKeyboardFunc(tracking_window_keyboard);
	glutKeyboardUpFunc(tracking_window_keyboard_up);
	glutSpecialFunc(tracking_window_keyboard_special);
	glutSpecialUpFunc(tracking_window_keyboard_special_up);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// myinit (called after glutMainLoop)
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(5.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(xmin, xmax, ymin, ymax);

}

void TrackingWindow::display()
{
	render();
#if 0
	glFlush();

	double currentTime = glutGet(GLUT_ELAPSED_TIME);
	nbFrames++;
	if (currentTime - lastTime >= 1000) 
	{
		printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		nbFrames = 0;
		lastTime += 1.0;
	}
#endif
}

void TrackingWindow::idle()
{
	render();
}

void TrackingWindow::render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(.40, .40, .40);
	glPointSize(15.0f);

	if (!_buffers_ready)
	{
		glGenBuffers(1, &_vbo_grid_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo_grid_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices_position), grid_vertices_position, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &_vbo_grid_indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_grid_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(grid_indices), grid_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenBuffers(1, &_vbo_grid_lines_indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_grid_lines_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(grid_lines_indices), grid_lines_indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		_buffers_ready = true;
	}
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScaled(_board_width, _board_height, 1.0);

	glEnableClientState(GL_VERTEX_ARRAY);

	// rendering grid points
	glBindBuffer(GL_ARRAY_BUFFER, _vbo_grid_vertices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_grid_indices);
	glVertexPointer(2, GL_FLOAT, 0, (void*)0);
	glDrawElements(GL_POINTS, 9, GL_UNSIGNED_INT, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_grid_lines_indices);
	glDrawElements(GL_LINES, 12, GL_UNSIGNED_INT, (void*)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// rendering grid corners

	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();

	if (_event.num_hands() > 0)
	{
		glEnable(GL_POINT_SMOOTH);
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_POINTS);
		glVertex2f(_event.x, _event.y);
		glEnd();
		glDisable(GL_POINT_SMOOTH);
	}

	glutSwapBuffers();
}

void TrackingWindow::keyboard(unsigned char key, int x, int y)
{
	_user_data.key.update(key, true);
}

void TrackingWindow::keyboard_up(unsigned char key, int x, int y)
{
	_user_data.key.update(key, false);
}

void TrackingWindow::keyboard_special(int key, int x, int y)
{
	_user_data.key.update(key, true);
}

void TrackingWindow::keyboard_special_up(int key, int x, int y)
{
	_user_data.key.update(key, false);
}

void TrackingWindow::update(const TrackingEvent& event)
{
	std::lock_guard<std::mutex> guard(_mutex);
	_event = event;
	if(glutGetWindow())
		glutPostRedisplay();
}
