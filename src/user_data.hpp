#pragma once

#include "coordinate_system.hpp"
#include "gesture_cue.hpp"
#include "key_state.hpp"
#include "tqueue.hpp"
#include "tracking_logger.hpp"

class UserData
{
protected:
	size_t _current_cue;
	   
public:

	KeyState key;
	std::vector<GestureCue> cue;
	CoordinateSystem coordinate_system;
	TrackingEventLogger event_logger;
		
	UserData();

	bool load_cues(void);
	GestureCue& current_cue();
};
