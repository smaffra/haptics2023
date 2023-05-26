#include<GL/glew.h>
#include<GL/glut.h>
#include<GL/freeglut.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

//#include "accelerometer.hpp"
//#include "moving_average.hpp"

#include "ExampleConnection.h"
#include "tracking_event.hpp"
#include "tracking_emitter.hpp"
#include "tracking_window.hpp"
#include "tracking_logger.hpp"
#include "user_data.hpp"
#include "utils.hpp"

#define EMITTER

//
// Global state
//

UserData user_data;
TrackingWindow window(user_data);
std::atomic<bool> stop_event_logging = false;

//
// Leap motion connection 
//

void leap_motion_tracking_callback(const LEAP_TRACKING_EVENT* leap_tracking_event)
{
	// updating coordinate system
	TrackingEvent event;
	user_data.coordinate_system.update(leap_tracking_event, &event);

	// fetching pressed key
	event.key = user_data.key.state();

	// storing event
	user_data.event_logger.store_event(event);

	// updating tracking window
	window.update(event);
}

void connect_leap_motion(bool verbose = true)
{
	std::cout << "Connecting to Leapmotion" << std::endl;
	// registering callback
	ConnectionCallbacks.on_frame = leap_motion_tracking_callback;

	// openning connection
	OpenConnection();
	while (!IsConnected)
		millisleep(10); //wait a bit to let the connection complete
	if (verbose)
		std::cout << "Connected." << std::endl;
	LEAP_DEVICE_INFO* deviceProps = GetDeviceProperties();
	if (verbose && deviceProps)
		std::cout << "Using device " << deviceProps->serial << std::endl;
}

void disconnect_leap_motion(void)
{
	DestroyConnection();
	std::cout << "Leapmotion disconnected" << std::endl;
}

//
// Writer thread
//

void dump_event_file(std::string filename, size_t batch_size, bool verbose)
{
	std::cout << "Starting event logging" << std::endl;
	user_data.event_logger.open(filename.c_str());

	while (true)
	{
		if (stop_event_logging)
			break;		
		user_data.event_logger.dump_events(batch_size, verbose);
	}

	user_data.event_logger.flush();
	user_data.event_logger.close();
	std::cout << "Stopping event logging" << std::endl;
}

//
// Visualization thread
//

void tracking_window(int argc, char** argv)
{
	// Initializing window
	glutInit(&argc, argv);
	window.init(500, 500, -0.3, 0.3, -0.3, 0.3);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW! Aborting" << std::endl;
		exit(-1);
	}
	glutMainLoop();
}

//
// Main 
//

void main(int argc, char** argv) 
{
	std::string output_filename;
	if (argc == 2)
		output_filename = argv[1];
	else if (argc == 1)
		output_filename = default_output_filename();
	else
	{
		std::cout << "Usage: track <output filename>" << std::endl;
		exit(1);
	}
	std::cout << "Recording events to: " << output_filename << std::endl;

	// Loading gesture cues
	if (user_data.load_cues() == false)
	{
		std::cout << "Error loading gesture cue files. Aborting..." << std::endl;
		exit(1);
	}

	// Start event logging thread
	std::thread thread_event_logging(dump_event_file, output_filename, 100, false);
	   
	// Connecting leap motion
	connect_leap_motion();

	// Initializing emitter
#ifdef EMITTER
	// Starting emitter
	TrackingEmitter emitter(user_data);
	emitter.start();
#endif
	
#if 1
	// Initializing window
	glutInit(&argc, argv);
	window.init(500, 500, -0.3, 0.3, -0.3, 0.3);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW! Aborting" << std::endl;
		exit(-1);
	}
	glutMainLoop();
#else
	std::thread thread_tracking_window(tracking_window, argc, argv);
	thread_tracking_window.join();
#endif

	// Stopping emitter
#ifdef EMITTER
	emitter.stop();
#endif
	
	// Disconnecting leap motion
	disconnect_leap_motion();

	// Stopping event logging thread
	stop_event_logging = true;
	thread_event_logging.join();
	
	std::cout << "DONE!" << std::endl;
}
