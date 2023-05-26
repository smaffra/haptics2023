#include <UltrahapticsTimePointStreaming.hpp>
#include <UltrahapticsAlignment.hpp>

#include "ExampleConnection.h"
#include "tracking_emitter.hpp"
#include "user_data.hpp"

void emitter_callback(
	const Ultrahaptics::TimePointStreaming::Emitter& timepoint_emitter,
	Ultrahaptics::TimePointStreaming::OutputInterval& interval,
	const Ultrahaptics::TimePoint& submission_deadline,
	void* user_data_ptr)
{
	TrackingEmitter* te = (TrackingEmitter*)user_data_ptr;
	te->callback(interval);
}

TrackingEmitter::TrackingEmitter(UserData& p) : _user_data(p)
{
}

TrackingEmitter::~TrackingEmitter()
{
}

void TrackingEmitter::start(int control_point_count)
{
	_emitter.setMaximumControlPointCount(control_point_count);
	_emitter.setEmissionCallback(emitter_callback, (void*) this);
	_emitter.start();
}

void TrackingEmitter::stop()
{
	_emitter.stop();
}

void TrackingEmitter::callback(Ultrahaptics::TimePointStreaming::OutputInterval& interval)
{
	Transform tf;
	_user_data.coordinate_system.get_transform(tf);

	bool disabled =
		_user_data.key.state() == KeyState::NONE ||
		tf.num_hands() < 1;

	if (disabled)
	{
		GestureCue& cue = _user_data.current_cue();
		for (Ultrahaptics::TimePointStreaming::OutputInterval::iterator it = interval.begin(); it < interval.end(); ++it)
		{
			for (size_t p = 0; p < cue.num_control_points(); p++)
			it->persistentControlPoint(p).setIntensity(0.0f);
		}
	}
	else
	{
		GestureCue& cue = _user_data.current_cue();
		for (Ultrahaptics::TimePointStreaming::OutputInterval::iterator it = interval.begin(); it < interval.end(); ++it)
		{
			for (size_t p = 0; p < cue.num_control_points(); p++)
			{
				static Ultrahaptics::Vector3 vp;
				tf.apply(cue.current_position(p), vp);
				it->persistentControlPoint(p).setPosition(vp);
				it->persistentControlPoint(p).setIntensity(cue.current_intensity(p));
			}
			cue.next_time_point();
		}
	}
}
