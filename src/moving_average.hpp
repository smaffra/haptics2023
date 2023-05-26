#pragma once

#include <iostream>
#include <cstddef>
#include <cmath>
#include "tqueue.hpp"

class MovingAverage
{
protected:
    size_t _length;
    float _moving_average;
    CircularQueue<float> _queue;

public:
    MovingAverage(size_t length) : _length(length), _queue(length), _moving_average(0.0)
    {
    }

    virtual ~MovingAverage()
    {
    }

    void push(float value)
    {
        if(_queue.full())
        {
			float old = _queue.front();
            _queue.pop();
            _queue.push(value);
            _moving_average += (value - old) / (_length - 1);
        }
        else
        {
            _queue.push(value);
            _moving_average += value / (_length - 1);
        }        
    }

    float value(void)
    {
        if(_queue.full())
            return _moving_average;
        return std::nanf("");
    }

	void print_queue(void)
	{
		_queue.print();
	}
};

