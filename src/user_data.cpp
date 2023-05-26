#include "user_data.hpp"


UserData::UserData() : _current_cue(KeyState::NONE), event_logger(1000000)
{
}

bool UserData::load_cues(void)
{
	static const char num_cues = 13;
	static const char* cues[num_cues] = {
		"cues/tap_half.txt",
		"cues/up.txt",
		"cues/down.txt",
		"cues/left.txt",
		"cues/right.txt",		
		"cues/north_half.txt",
		"cues/northeast_half.txt",
		"cues/east_half.txt",
		"cues/southeast_half.txt",
		"cues/south_half.txt",
		"cues/southwest_half.txt",
		"cues/west_half.txt",
		"cues/northwest_half.txt",
	};

	cue.resize(num_cues);
	for (size_t i = 0; i < num_cues; i++)
	{
		if (cue[i].load(cues[i]) == false)
			return false;
	}
	return true;
}

GestureCue& UserData::current_cue()
{
	// same cue is currently playing
	if (_current_cue == key.state())
		return cue[_current_cue];

	// new cue is to be played
	_current_cue = key.state();
	cue[_current_cue].reset_time_point();
	return cue[_current_cue];
}

