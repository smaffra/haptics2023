#pragma once
#include "GL/glut.h"

#include <atomic>

class KeyState
{
protected:
	std::atomic<int> _state;

public:

	enum State : int {
		NONE = 0,
		UP = 1,
		DOWN = 2,
		LEFT = 3,
		RIGHT = 4,
		N = 5,
		NE = 6,
		E = 7,
		SE = 8,
		S = 9,
		SW = 10,
		W = 11,
		NW = 12
	};

	KeyState() : _state(NONE)
	{
	}

	int state()
	{
		return _state;
	}

	void update(int key, bool pressed)
	{
		if (pressed)
		{
			switch (key)
			{
			case '1':
				_state = State::SW;
				return;
			case '2':
				_state = State::S;
				return;
			case '3':
				_state = State::SE;
				return;
			case '4':
				_state = State::W;
				return;
			//case 5:
			//	_state = State::N;
			//	return;
			case '6':
				_state = State::E;
				return;
			case '7':
				_state = State::NW;
				return;
			case '8':
				_state = State::N;
				return;
			case '9':
				_state = State::NE;
				return;
			case GLUT_KEY_UP:
				_state = State::UP;
				return;
			case GLUT_KEY_DOWN:
				_state = State::DOWN;
				return;
			case GLUT_KEY_LEFT:
				_state = State::LEFT; 
				return;
			case GLUT_KEY_RIGHT: 
				_state = State::RIGHT; 
				return;
			};
		}
		_state = State::NONE;
		return;
	}

	void reset()
	{
		_state = State::NONE;
	}
};
