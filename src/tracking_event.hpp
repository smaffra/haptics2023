#pragma once

#include <cstdint>
#include <cmath>

struct TrackingEvent
{
	int64_t time;
	int key;
	float x;
	float y;
	bool right_hand;
	bool left_hand;

	int num_hands(void)
	{
		return int(right_hand) + int(left_hand);
	}

	bool operator==(const TrackingEvent& e)
	{
		if (key != e.key)
			return false;
		if (fabs(x - e.x) >= 1e-3)
			return false;
		if (fabs(y - e.y) >= 1e-3)
			return false;
		return true;
	}
};
