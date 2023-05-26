#pragma once

#include <UltrahapticsTimePointStreaming.hpp>

class UserData;

class TrackingEmitter
{
protected:
	UserData& _user_data;
	Ultrahaptics::TimePointStreaming::Emitter _emitter;
	
public:
	TrackingEmitter(UserData&);
	virtual ~TrackingEmitter();
	void start(int control_point_count=1);
	void stop();

	void callback(Ultrahaptics::TimePointStreaming::OutputInterval& interval);
};
