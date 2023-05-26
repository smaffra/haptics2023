#pragma once

#include <cstddef>
#include <chrono>
#include <mutex>
#include <Ultrahaptics.hpp>

#include "tracking_event.hpp"


extern "C"
{
#include "LeapC.h"
}


struct Transform
{
	int64_t time;
	int64_t tracking_frame_id;
	float palm_width;
	float palm_height;
	Ultrahaptics::Vector3 O;
	Ultrahaptics::Vector3 X;
	Ultrahaptics::Vector3 Y;
	bool right_hand;
	bool left_hand;

	Transform() : time(0), palm_width(0.0f), palm_height(0.0f), tracking_frame_id(0), right_hand(false), left_hand(false)
	{
	}

	void apply(Ultrahaptics::Vector3& v, Ultrahaptics::Vector3& vout)
	{
		float x = palm_width * v.x;
		float y = palm_height * v.y;
		vout.x = O.x + (x * X.x) + (y * Y.x);
		vout.y = O.y + (x * X.y) + (y * Y.y);
		vout.z = O.z + (x * X.z) + (y * Y.z);
	}

	int num_hands(void)
	{
		return int(right_hand) + int(left_hand);
	}
};


class CoordinateSystem
{
protected:

	Ultrahaptics::Alignment alignment;
	Ultrahaptics::Vector3 D;
	Ultrahaptics::Vector3 N;
	Ultrahaptics::Vector3 Vu;
	Transform _transform;
	std::mutex _mutex;
	
public:

	void update(const LEAP_TRACKING_EVENT* frame, TrackingEvent* event=nullptr)
	{
		std::lock_guard<std::mutex> guard(_mutex);

		// checking if a new frame is available
		if (!frame || frame->tracking_frame_id <= _transform.tracking_frame_id)
			return;
		_transform.tracking_frame_id = frame->tracking_frame_id;

		// fetching current time
		std::chrono::high_resolution_clock::duration dt = std::chrono::high_resolution_clock::now().time_since_epoch();
		int64_t current_time = std::chrono::duration_cast<std::chrono::milliseconds>(dt).count();
		
		if (frame->nHands > 0)
		{
			// updating coordinate system
			LEAP_HAND* hand = &frame->pHands[0];
			// palm position
			_transform.O.x = hand->palm.position.x;
			_transform.O.y = hand->palm.position.y;
			_transform.O.z = hand->palm.position.z;
			// palm direction (center to fingers)
			D.x = hand->palm.direction.x;
			D.y = hand->palm.direction.y;
			D.z = hand->palm.direction.z;
			// palm normal
			N.x = -hand->palm.normal.x;
			N.y = -hand->palm.normal.y;
			N.z = -hand->palm.normal.z;
			// basis
			_transform.palm_width = hand->palm.width / 10;
			_transform.palm_height = _transform.palm_width;
			// Convert from leap space to device space.
			_transform.O = alignment.fromTrackingPositionToDevicePosition(_transform.O);
			N = alignment.fromTrackingDirectionToDeviceDirection(N).normalize();
			_transform.Y = alignment.fromTrackingDirectionToDeviceDirection(D).normalize();
			_transform.X = N.cross(_transform.Y);

			// storing current time
			_transform.time = current_time;

			// hand count information
			_transform.left_hand = frame->pHands[0].type == eLeapHandType_Left;
			_transform.right_hand = frame->pHands[0].type == eLeapHandType_Right;

			if (event)
			{
				event->time = _transform.time;
				event->x = _transform.O.x;
				event->y = _transform.O.y;
				event->left_hand = _transform.left_hand;
				event->right_hand = _transform.right_hand;
			}
		}
		else
		{
			_transform.time = current_time;
			_transform.left_hand = false;
			_transform.right_hand = false;
			
			if (event)
			{
				event->time = _transform.time;
				event->x = 0.0;
				event->y = 0.0;
				event->left_hand = false;
				event->right_hand = false;
			}
		}
	}

	void get_transform(struct Transform& t)
	{
		std::lock_guard<std::mutex> guard(_mutex);
		t = _transform;
	}

	void get_timed_position(Ultrahaptics::Vector3& v, int64_t& time)
	{
		std::lock_guard<std::mutex> guard(_mutex);
		v = _transform.O;
		time = _transform.time;
	}
};
