#pragma once

#include <cstddef>


template<typename T> 
class CircularQueue
{
protected:

	size_t _head;
	size_t _tail;
	size_t _length;
	size_t _count;
	T* _data;

	size_t next_tail()
	{
		return (_tail + 1) % _length;
	}

	size_t next_head()
	{
		return (_head + 1) % _length;
	}

public:

	CircularQueue(size_t length) : _head(0), _tail(0), _length(length), _count(0)
	{
		_data = new T[_length];
	}

	virtual ~CircularQueue()
	{
		delete[] _data;
	}
	   
	bool empty(void)
	{
		return _head == _tail;
	}

	bool full(void)
	{
		return next_tail() == _head;
	}

	bool push(const T& obj)
	{
		if (full())
			return false;
		_data[_tail] = obj;
		_tail = next_tail();
		_count++;
		return true;
	}

	bool pop(void)
	{
		if (empty())
			return false;
		_head = next_head();
		_count--;
		return true;
	}

	T& front(void)
	{
		return _data[_head];
	}

	T& back(void)
	{
		if (_tail == 0)
			return _data[_length - 1];
		return _data[_tail - 1];
	}

	size_t count()
	{
		return _count;
	}

	void print(void)
	{
		for (size_t i = _head; i != _tail; i = (i + 1) % _length)
			std::cout << _data[i] << ' ';
		std::cout << std::endl;
	}

	void print_buffer(void)
	{
		for (size_t i = 0; i < _length; i++)
			std::cout << _data[i] << ' ';
		std::cout << std::endl;
	}
};

