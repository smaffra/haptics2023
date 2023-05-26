#include <cstdio>
#include "tqueue.hpp"
#include "moving_average.hpp"
#include "accelerometer.hpp"
#include <iostream>


int main(void)
{
	//MovingAverage ma(10);
	//Accelerometer accel;

	CircularQueue<int> q(5);

	for (int i = 0; i < 30; i++)
	{
		if (q.full())
		{
			q.pop();
			q.push(i);
		}
		else
		{
			q.push(i);
		}
		q.print();
	}

	return 0;
#if 0
	for (int i = 0; i < 5; i++)
	{
		if (!q.full())
			q.push(i + 1);
	}

	q.print_buffer();

	q.print();

	std::cout << "front: " << q.front() << std::endl;
	std::cout << "back: " << q.back() << std::endl;

	for (int i = 0; i < 10; i++)
		q.pop();

	q.push(i + 1);
	std::cout
		<< i
		<< " front: " << q.front()
		<< " back: " << q.back()
		<< " full? " << int(q.full())
		<< " empty? " << int(q.empty())
		<< std::endl;

	std::cout << "q:" << std::endl;
	q.print();

	for (int i = 0; i < 4; i++)
	{
		q.pop();
	}

	//q.print();

	q.push(10);
	q.push(100);
	q.push(1000);

	//q.print();

	//while (q.empty() == false) {
	//	std::cout << q.front() << std::endl;
	//	q.pop();
	//}

	printf("Alo mundo!");

#endif
}
