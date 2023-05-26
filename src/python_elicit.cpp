#include <boost/python/def.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/list.hpp>
#include <boost/python/module.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/numpy/ndarray.hpp>
#include <boost/python/object.hpp>
#include <cstdio>
#include <iostream>
#include <thread>
#include <UltrahapticsTimePointStreaming.hpp>
#include <UltrahapticsAlignment.hpp>
#include <vector>

#ifdef _LEAP_MOTION_321_
#include <Leap.h>
#else
#include <LeapC.h>
#include "ExampleConnection.h"
#endif

#define CAPPED false
#define FILTER false

class Emitter;
void emitter_loop_callback(
	const Ultrahaptics::TimePointStreaming::Emitter& timepoint_emitter,
	Ultrahaptics::TimePointStreaming::OutputInterval& interval,
	const Ultrahaptics::TimePoint& submission_deadline,
	void* user_data_ptr);

void emitter_callback(
	const Ultrahaptics::TimePointStreaming::Emitter& timepoint_emitter,
	Ultrahaptics::TimePointStreaming::OutputInterval& interval,
	const Ultrahaptics::TimePoint& submission_deadline,
	void* user_data_ptr);


#ifdef _LEAP_MOTION_321_

// Structure to represent output from the Leap listener
struct LeapOutput
{
	LeapOutput() noexcept {}

	Ultrahaptics::Vector3 palm_position;
	Ultrahaptics::Vector3 x_axis;
	Ultrahaptics::Vector3 y_axis;
	Ultrahaptics::Vector3 z_axis;
	bool hand_present = false;
};


// Leap listener class - tracking the hand position and creating data structure for use by Ultrahaptics API
// https://developer-archive.leapmotion.com/documentation/v2/cpp/api/Leap.Hand.html#cppclass_leap_1_1_hand
class LeapListening : public Leap::Listener
{
public:
	LeapListening(const Ultrahaptics::Alignment& align) : alignment(align)
	{
	}

	~LeapListening() = default;

	LeapListening(const LeapListening& other) = delete;
	LeapListening& operator=(const LeapListening& other) = delete;

	void onFrame(const Leap::Controller& controller) override
	{
		std::cout << "LeapListening::onFrame" << std::endl;
		// Get all the hand positions from the leap and position a focal point on each.
		const Leap::Frame frame = controller.frame();
		const Leap::HandList hands = frame.hands();
		LeapOutput local_hand_data;

		if (hands.isEmpty())
		{
			local_hand_data.palm_position = Ultrahaptics::Vector3();
			local_hand_data.x_axis = Ultrahaptics::Vector3();
			local_hand_data.y_axis = Ultrahaptics::Vector3();
			local_hand_data.z_axis = Ultrahaptics::Vector3();
			local_hand_data.hand_present = false;
		}
		else
		{
			Leap::Hand hand = hands[0];

			// Translate the hand position from leap objects to Ultrahaptics objects.
			Leap::Vector leap_palm_position = hand.palmPosition();
			Leap::Vector leap_palm_normal = hand.palmNormal();
			Leap::Vector leap_palm_direction = hand.direction();

			// Convert to Ultrahaptics vectors, normal is negated as leap normal points down.
			Ultrahaptics::Vector3 uh_palm_position(leap_palm_position.x, leap_palm_position.y, leap_palm_position.z);
			Ultrahaptics::Vector3 uh_palm_normal(-leap_palm_normal.x, -leap_palm_normal.y, -leap_palm_normal.z);
			Ultrahaptics::Vector3 uh_palm_direction(leap_palm_direction.x, leap_palm_direction.y, leap_palm_direction.z);

			// Convert from leap space to device space.
			Ultrahaptics::Vector3 device_palm_position = alignment.fromTrackingPositionToDevicePosition(uh_palm_position);
			Ultrahaptics::Vector3 device_palm_normal = alignment.fromTrackingDirectionToDeviceDirection(uh_palm_normal).normalize();
			Ultrahaptics::Vector3 device_palm_direction = alignment.fromTrackingDirectionToDeviceDirection(uh_palm_direction).normalize();

			// Check palm position is within 14cm either side of the centre of array.
			if (device_palm_position.x >= -14.0f * Ultrahaptics::Units::cm && device_palm_position.x <= 14.0f * Ultrahaptics::Units::cm)
			{
				// These can then be converted to be a unit axis on the palm of the hand.
				Ultrahaptics::Vector3 device_palm_z = device_palm_normal;                             // Unit Z direction.
				Ultrahaptics::Vector3 device_palm_y = device_palm_direction;                          // Unit Y direction.
				Ultrahaptics::Vector3 device_palm_x = device_palm_y.cross(device_palm_z).normalize(); // Unit X direction.

				local_hand_data.palm_position = device_palm_position;
				local_hand_data.x_axis = device_palm_x;
				local_hand_data.y_axis = device_palm_y;
				local_hand_data.z_axis = device_palm_z;
				local_hand_data.hand_present = true;
			}
			else
			{
				local_hand_data.palm_position = Ultrahaptics::Vector3();
				local_hand_data.x_axis = Ultrahaptics::Vector3();
				local_hand_data.y_axis = Ultrahaptics::Vector3();
				local_hand_data.z_axis = Ultrahaptics::Vector3();
				local_hand_data.hand_present = false;
			}
		}

		std::cout << "palm position: " << local_hand_data.palm_position << std::endl;


		atomic_local_hand_data.store(local_hand_data);
	}

	LeapOutput getLeapOutput()
	{
		return atomic_local_hand_data.load();
	}

private:
	std::atomic <LeapOutput> atomic_local_hand_data;
	Ultrahaptics::Alignment alignment;
};

#endif

struct UserData
{
	UserData(const unsigned timepointCount, const unsigned controlpointCount, Emitter* emitter = nullptr) :
		next_index(0),
		loop_render_mode(false),
		timepointCount(timepointCount),
		controlpointCount(controlpointCount),
		_emitter(emitter),
		palm_height(1.0),
		palm_width(1.0),
		lastFrameId(0),
		timepoints(
			std::vector<std::vector<Ultrahaptics::TimePointStreaming::ControlPoint>>(timepointCount, std::vector<Ultrahaptics::TimePointStreaming::ControlPoint>(
				controlpointCount)
				)
		)
	{}

	void reset()
	{
		next_index = 0;
	}

	bool render_complete()
	{
		if (loop_render_mode)
		{
			return false;
		}
		else // single render mode
		{
			return next_index >= timepoints.size();
		}
	}

	bool next()
	{
		if (loop_render_mode)
		{
			next_index = (next_index + 1) % timepoints.size();
		}
		else
		{
			if (next_index > timepoints.size())
				return false;
			next_index++;
		}
		return true;
	}

	inline void update_coordinate_system(void)
	{
		LEAP_TRACKING_EVENT* frame = GetFrame();
		if (frame && frame->tracking_frame_id > lastFrameId)
		{
			lastFrameId = frame->tracking_frame_id;

			LEAP_HAND* hand = &frame->pHands[0];
			// palm position
			O.x = hand->palm.position.x;
			O.y = hand->palm.position.y;
			O.z = hand->palm.position.z;
			// palm direction (center to fingers)
			D.x = hand->palm.direction.x;
			D.y = hand->palm.direction.y;
			D.z = hand->palm.direction.z;
			// palm normal
			N.x = -hand->palm.normal.x;
			N.y = -hand->palm.normal.y;
			N.z = -hand->palm.normal.z;
			// basis
			if (palm_size_tracking_enabled)
			{
				palm_width = hand->palm.width / 10;
				palm_height = palm_width;
				//std::cout << "palm width " << palm_width << std::endl;
			}
			// Convert from leap space to device space.
			O = alignment.fromTrackingPositionToDevicePosition(O);
			N = alignment.fromTrackingDirectionToDeviceDirection(N).normalize();
			Y = alignment.fromTrackingDirectionToDeviceDirection(D).normalize();
			X = N.cross(Y);
		}
	}

	inline void transform(float x, float y, float z, Ultrahaptics::Vector3& v)
	{
		x = palm_width * x;
		y = palm_height * y;
		v.x = O.x + (x * X.x) + (y * Y.x);
		v.y = O.y + (x * X.y) + (y * Y.y);
		v.z = O.z + (x * X.z) + (y * Y.z);
	}

	const unsigned timepointCount;
	const unsigned controlpointCount;
	unsigned next_index;
	std::vector<std::vector<Ultrahaptics::TimePointStreaming::ControlPoint>> timepoints;
	bool loop_render_mode;
	bool position_tracking_enabled;
	bool palm_size_tracking_enabled;
	Emitter* _emitter;

	Ultrahaptics::Alignment alignment;
	Ultrahaptics::Vector3 O;
	Ultrahaptics::Vector3 D;
	Ultrahaptics::Vector3 N;
	Ultrahaptics::Vector3 X;
	Ultrahaptics::Vector3 Y;
	Ultrahaptics::Vector3 Vu;
	float palm_width;
	float palm_height;
	int64_t lastFrameId;
};


class Emitter
{
public:

	UserData* _user_data;
	int _hardware_sample_rate;
	bool _filter;
	bool _capped;
	bool _loop_render_mode;
	bool _position_tracking_enabled;
	bool _palm_size_tracking_enabled;
	float _palm_width;
	float _palm_height;
	Ultrahaptics::TimePointStreaming::Emitter* _emitter;
#ifdef _LEAP_MOTION_321_
	LeapListening* _leap_listener;
	Leap::Controller _leap_controller;
#endif
	bool _started;

	Emitter() :
		_user_data(nullptr), _hardware_sample_rate(0), _filter(false), _capped(false), 
		_loop_render_mode(false), _position_tracking_enabled(false), _palm_size_tracking_enabled(false),
		_palm_width(1.0f), _palm_height(1.0f),
		_emitter(nullptr), 
#ifdef _LEAP_MOTION_321_
		_leap_listener(nullptr), 
#endif
		_started(false)
	{
#ifdef _LEAP_MOTION_321_
		_leap_controller.setPolicyFlags(Leap::Controller::PolicyFlag::POLICY_BACKGROUND_FRAMES);	
		//_leap_controller.config().setBool("power_saving_adapter", false);
		//_leap_controller.config().save();
#endif
	}

	virtual ~Emitter() 
	{
#ifdef _LEAP_MOTION_321_
		destroy_leap_listener();
#endif
		destroy_emitter();

		if (_user_data != nullptr)
			delete _user_data;
	}

#ifdef _LEAP_MOTION_321_
	void destroy_leap_listener()
	{
		if (_leap_listener != nullptr)
		{
			_leap_controller.removeListener(*_leap_listener);
			delete _leap_listener;
			_leap_listener = nullptr;
		}
	}
#endif

	void leap_connect()
	{
		OpenConnection();
		while (!IsConnected)
			millisleep(100);
		printf("Leapmotion connected.");
		LEAP_DEVICE_INFO* deviceProps = GetDeviceProperties();
		if (deviceProps)
			printf("Using device %s.\n", deviceProps->serial);
	}

	void destroy_emitter()
	{
		if (_emitter)
		{
			_emitter->stop();
			delete _emitter;
			_emitter = nullptr;
		}
	}

	void stop()
	{
		_started = false;
		destroy_emitter();
#ifdef _LEAP_MOTION_321_
		destroy_leap_listener();
#endif
	}

	void start(
		boost::python::object loop_render_mode, 
		boost::python::object position_tracking_enabled, 
		boost::python::object palm_size_tracking_enabled,
		boost::python::object palm_width,
		boost::python::object palm_height)
	{
		if (_started) 
		{
			std::cout << "Emitter::start> Emitter already started!" << std::endl;
			return;
		}

		_loop_render_mode = boost::python::extract<bool>(loop_render_mode);
		_position_tracking_enabled = boost::python::extract<bool>(position_tracking_enabled);
		_palm_size_tracking_enabled = boost::python::extract<bool>(palm_size_tracking_enabled);
		_palm_width = boost::python::extract<float>(palm_width);
		_palm_height = boost::python::extract<float>(palm_height);

		_emitter = new Ultrahaptics::TimePointStreaming::Emitter();
		std::cout << "Connected" << std::endl;

		// Print device ID and firmware version
		std::cout << "Device ID: " << _emitter->getDeviceInfo().getDeviceIdentifier() << std::endl;
		std::cout << "Firmware version: " << _emitter->getDeviceInfo().getFirmwareVersion() << std::endl;

		// Print software version
		char buf[128] = { 0 };
		_emitter->getExtendedOption("softwareVersion", buf, 128);
		std::cout << "Software version: " << buf << std::endl;

		// setting number of control points
		unsigned int hardware_sample_rate = _emitter->setMaximumControlPointCount(_user_data->controlpointCount);
		
		// check sample rate
		//_emitter->setSampleRate(_hardware_sample_rate);
		//std::cout << "Sample rate:" << _emitter->getSampleRate() << std::endl;

		//if (!_capped) {
		//	_emitter->setExtendedOption("solverCappingConstant", "3000");
		//	std::cout << "Capping OFF" << std::endl;
		//}
		//else
		//	std::cout << "Capping ON" << std::endl;

		//if (!_filter) {
		//	_emitter->setExtendedOption("setFilterCutoffFrequencies", "0 0 0 0");
		//	std::cout << "Filtering OFF" << std::endl;
		//}
		//else
		std::cout << "Filtering ON" << std::endl;

#ifdef _LEAP_MOTION_321_
		destroy_leap_listener();
		_leap_listener = new LeapListening(_emitter->getDeviceInfo().getDefaultAlignment());
		_leap_controller.addListener(*_leap_listener);
		std::cout << "Leapmotion listener added!" << std::endl;
#else
		leap_connect();
#endif

		// Start the array
		_user_data->loop_render_mode = _loop_render_mode;
		_user_data->position_tracking_enabled = _position_tracking_enabled;
		_user_data->palm_size_tracking_enabled = _palm_size_tracking_enabled;
		_user_data->palm_height = _palm_height;
		_user_data->palm_width = _palm_width;
		_user_data->reset();
		// storing alignment object
		_user_data->alignment = _emitter->getDeviceInfo().getDefaultAlignment();
		_emitter->setEmissionCallback(emitter_callback, _user_data);
		_emitter->start();
		_started = true;
	}

	bool loop_render_mode() 
	{
		return _loop_render_mode;
	}

	void load_control_points(boost::python::list l, boost::python::object hardware_sample_rate, boost::python::object capped, boost::python::object filter)
	{
		using namespace boost::python;

		_hardware_sample_rate = extract<int>(hardware_sample_rate);
		_capped = extract<bool>(capped);
		_filter = extract<bool>(filter);

		int time_point_count = 0;
		int control_point_count = len(l);

		// checking the maximum number of rows (timepoints)
		for (long j = 0; j < control_point_count; j++)
		{
			printf("element: %d\n", j);
			object cp = l[j];
			numpy::ndarray a = extract<numpy::ndarray>(cp);
			//print_array(a);

			int nd = a.get_nd();
			int nrows = a.shape(0);
			int ncols = a.shape(1);

			if (nrows > time_point_count)
				time_point_count = nrows;
		}

		std::cout << "time_point_count: " << time_point_count << std::endl;
		std::cout << "control_point_count: " << control_point_count << std::endl;

		if (_user_data == nullptr)
			delete _user_data;

		_user_data = new UserData(time_point_count, control_point_count, this);
		std::cout << " timepoints.size: " << _user_data->timepoints.size() << std::endl;
		std::cout << " timepoint[0].size: " << _user_data->timepoints[0].size() << std::endl;

		for (long j = 0; j < control_point_count; j++)
		{
			std::cout << "j=" << j << std::endl;
			numpy::ndarray a = extract<numpy::ndarray>(l[j]);
			for (long i = 0; i < time_point_count; i++)
			{
				float x = extract<double>(a[i][0]);
				float y = extract<double>(a[i][1]);
				float z = extract<double>(a[i][2]);
				float w = extract<double>(a[i][3]);
				_user_data->timepoints[i][j].intensity = w;
				_user_data->timepoints[i][j].position = Ultrahaptics::Vector3(x, y, z);
			}
		}
		std::cout << "Loaded data into Emitter!" << std::endl;
	}
};


#if 0

void emitter_loop_callback(
	const Ultrahaptics::TimePointStreaming::Emitter& timepoint_emitter,
	Ultrahaptics::TimePointStreaming::OutputInterval& interval,
	const Ultrahaptics::TimePoint& submission_deadline,
	void* user_data_ptr) 
{
	UserData* user_data = static_cast<UserData*>(user_data_ptr);
	double intensity_multiplier = 1.0;

	//LeapOutput leap_output = user_data->_emitter->_leap_listener->getLeapOutput();
	//if (!leap_output.hand_present)
	//{
	//	intensity_multiplier = 0.0;
	//}
	//else 
	//{
	//    std::cout << "palm position: " << leap_output.palm_position << std::endl;
	//}

	for (Ultrahaptics::TimePointStreaming::OutputInterval::iterator it = interval.begin(); it < interval.end(); ++it)
	{
		for (size_t i = 0; i < user_data->controlpointCount; i++)
		{
			it->persistentControlPoint(i).setPosition(user_data->timepoints[user_data->next_index][i].position);
			it->persistentControlPoint(i).setIntensity(intensity_multiplier * user_data->timepoints[user_data->next_index][i].intensity);
		}
		user_data->next_index++;
		user_data->next_index = user_data->next_index % user_data->timepoints.size();
		//user_data->next_index = (user_data->next_index + unsigned(1)) % user_data->timepoints.size();

#if 0
		std::cout
			<< user_data->timepoints[user_data->next_index][0].position.x << ' '
			<< user_data->timepoints[user_data->next_index][0].position.y << ' '
			<< user_data->timepoints[user_data->next_index][0].position.z << ' '
			<< user_data->timepoints[user_data->next_index][0].intensity << ' '
			<< " timepoints: " << int(user_data->timepoints.size())
			<< " float inc: " << user_data->next_index
			<< " intensity: " << user_data->timepoints[user_data->next_index][0].getIntensity() << std::endl;
#endif
	}
}

void emitter_callback_old(
	const Ultrahaptics::TimePointStreaming::Emitter& timepoint_emitter,
	Ultrahaptics::TimePointStreaming::OutputInterval& interval,
	const Ultrahaptics::TimePoint& submission_deadline,
	void* user_data_ptr)
{
	UserData* user_data = static_cast<UserData*>(user_data_ptr);

	for (Ultrahaptics::TimePointStreaming::OutputInterval::iterator it = interval.begin(); it < interval.end(); ++it)
	{
		if (user_data->render_complete())
		{
			for (size_t i = 0; i < user_data->controlpointCount; i++)
			{
				//it->persistentControlPoint(i).setPosition(user_data->timepoints[0][i].position);
				it->persistentControlPoint(i).setIntensity(0.0);
			}
		}
		else
		{
			for (size_t i = 0; i < user_data->controlpointCount; i++)
			{
				it->persistentControlPoint(i).setPosition(user_data->timepoints[user_data->next_index][i].position);
				it->persistentControlPoint(i).setIntensity(user_data->timepoints[user_data->next_index][i].intensity);
			}
			user_data->next();
		}
	}
}

#endif

void emitter_callback(
	const Ultrahaptics::TimePointStreaming::Emitter& timepoint_emitter,
	Ultrahaptics::TimePointStreaming::OutputInterval& interval,
	const Ultrahaptics::TimePoint& submission_deadline,
	void* user_data_ptr)
{
	UserData* user_data = static_cast<UserData*>(user_data_ptr);

	if (user_data->position_tracking_enabled)
	{
		user_data->update_coordinate_system();
		for (Ultrahaptics::TimePointStreaming::OutputInterval::iterator it = interval.begin(); it < interval.end(); ++it) 
		{
			if (user_data->render_complete())
			{
				for (size_t i = 0; i < user_data->controlpointCount; i++)
					it->persistentControlPoint(i).setIntensity(0.0);
			}
			else
			{
				for (size_t i = 0; i < user_data->controlpointCount; i++)
				{
					static Ultrahaptics::Vector3 V;
					user_data->transform(
						user_data->timepoints[user_data->next_index][i].position.x,
						user_data->timepoints[user_data->next_index][i].position.y,
						user_data->timepoints[user_data->next_index][i].position.z,
						V);
					it->persistentControlPoint(i).setPosition(V);
					it->persistentControlPoint(i).setIntensity(user_data->timepoints[user_data->next_index][i].intensity);
				}
				user_data->next();
			}
		}
	}
	else
	{
		for (Ultrahaptics::TimePointStreaming::OutputInterval::iterator it = interval.begin(); it < interval.end(); ++it)
		{
			if (user_data->render_complete())
			{
				for (size_t i = 0; i < user_data->controlpointCount; i++)
					it->persistentControlPoint(i).setIntensity(0.0);
			}
			else
			{
				for (size_t i = 0; i < user_data->controlpointCount; i++)
				{
					it->persistentControlPoint(i).setPosition(user_data->timepoints[user_data->next_index][i].position);
					it->persistentControlPoint(i).setIntensity(user_data->timepoints[user_data->next_index][i].intensity);
				}
				user_data->next();
			}
		}
	}
}


BOOST_PYTHON_MODULE(python_haptics)
{
	using namespace boost::python;
	numpy::initialize();

	class_<Emitter>("Emitter")
		.def("load_control_points", &Emitter::load_control_points)
		.def("start", &Emitter::start)
		.def("stop", &Emitter::stop)
		.def("loop_render_mode", &Emitter::loop_render_mode)
	;
}

