// This example uses the Timepoint Streaming emitter and a Leap Motion Controller.
// It projects a forcefield along the array's X axis onto the palm.

#include <atomic>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <Leap.h>

#ifndef M_PI
#define M_PI 3.14159265358979323
#endif

//#include "WaveformData.hpp"
#include "UltrahapticsTimePointStreaming.hpp"

// Structure to represent output from the Leap listener
struct LeapOutput
{
    LeapOutput() noexcept {}

    Ultrahaptics::Vector3 palm_position;
    Ultrahaptics::Vector3 x_axis;
    bool hand_present = false;
};

// Leap listener class - tracking the hand position and creating data structure for use by Ultrahaptics API
class LeapListening : public Leap::Listener
{
public:
    LeapListening(const Ultrahaptics::Alignment& align)
      : alignment(align)
    {
    }

    ~LeapListening() = default;

    LeapListening(const LeapListening &other) = delete;
    LeapListening &operator=(const LeapListening &other) = delete;

    void onFrame(const Leap::Controller &controller) override
    {
		std::cout << "onframe" << std::endl;
        // Get all the hand positions from the leap and position a focal point on each.
        const Leap::Frame frame = controller.frame();
        const Leap::HandList hands = frame.hands();
        LeapOutput local_hand_data;

        if (hands.isEmpty())
        {
            local_hand_data.palm_position = Ultrahaptics::Vector3();
            local_hand_data.x_axis = Ultrahaptics::Vector3();
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
                local_hand_data.hand_present = true;
            }
            else
            {
                local_hand_data.palm_position = Ultrahaptics::Vector3();
                local_hand_data.x_axis = Ultrahaptics::Vector3();
                local_hand_data.hand_present = false;
            }
        }
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

// Structure for passing information on the type of point to create
struct ModulatedPoint
{
    // Create the structure, passing the appropriate alignment through to the LeapListening class
    ModulatedPoint(const Ultrahaptics::Alignment& align) : hand(align)
    {
    }

    // Hand data
    LeapListening hand;

    // The position of the control point
    Ultrahaptics::Vector3 position;

    // The width of the forcefield
    double forcefield_width;

    // The frequency at the forcefield is created
    double forcefield_frequency;

    // The offset of the control point at the last sample time
    double offset = 0.0;

    // This allows us to easily reverse the direction of the point.
    // It is 1 or -1.
    int direction = 1;
};

// Callback function for filling out complete device output states through time
void my_emitter_callback(const Ultrahaptics::TimePointStreaming::Emitter &timepoint_emitter,
                         Ultrahaptics::TimePointStreaming::OutputInterval &interval,
                         const Ultrahaptics::HostTimePoint &submission_deadline,
                         void *user_pointer)
{
	//std::cout << " emitter " << std::endl;
    // Cast the user pointer to the struct that describes the control point behaviour
    ModulatedPoint *my_modulated_point = static_cast<ModulatedPoint*>(user_pointer);

    // Set interval offset between control point samples
    double interval_offset = my_modulated_point->forcefield_width * my_modulated_point->forcefield_frequency / timepoint_emitter.getSampleRate();

    // Get a copy of the Leap data
    LeapOutput leapOutput = my_modulated_point->hand.getLeapOutput();

    // Loop through time, setting control points
    for (auto& sample : interval)
    {
        if (!leapOutput.hand_present) {
            // no hand -> don't output anything
            sample.persistentControlPoint(0).setIntensity(0.0f);
            continue;
        }

        double os = my_modulated_point->offset;
        // Point moves back and forth along the x-axis of the palm, forming a very thin rectangle with a small y-axis variation
        my_modulated_point->position.x = leapOutput.palm_position.x + (my_modulated_point->direction * os);
        my_modulated_point->position.y = my_modulated_point->direction * 5.0f * Ultrahaptics::Units::mm;
        my_modulated_point->position.z = (os * leapOutput.x_axis.z) + leapOutput.palm_position.z;

        sample.persistentControlPoint(0).setPosition(my_modulated_point->position);
        sample.persistentControlPoint(0).setIntensity(1.0f);

        // Update previous sample offset for next loop iteration
        my_modulated_point->offset += interval_offset * my_modulated_point->direction;

        // Check if we reached the edge of the forcefield projection width, and reverse direction if so
        if (fabs(my_modulated_point->offset) > my_modulated_point->forcefield_width / 2)
        {
            // Toggle the direction.
            my_modulated_point->direction = -my_modulated_point->direction;
        }
    }
}

int main(int argc, char *argv[])
{
    // Create a time point streaming emitter and a leap controller
    Ultrahaptics::TimePointStreaming::Emitter emitter;
    Leap::Controller leap_control;

    // Set the leap motion to listen for background frames.
    leap_control.setPolicyFlags(Leap::Controller::PolicyFlag::POLICY_BACKGROUND_FRAMES);

    // We will be using 1 control point
    emitter.setMaximumControlPointCount(1);

    // Create a structure containing our control point data
    // Also pass the Leap conversion class the appropriate alignment for the device we're using
    ModulatedPoint point(emitter.getDeviceInfo().getDefaultAlignment());

    // Update our structure with data from our Leap listener
    leap_control.addListener(point.hand);

    // Set the width of the forcefield that the point is traversing
    point.forcefield_width = 10.0 * Ultrahaptics::Units::centimetres;

    // Set how many times the point traverses the forcefield every second
    point.forcefield_frequency = 100.0;

    // Set the callback function to the callback written above
    emitter.setEmissionCallback(my_emitter_callback, &point);

    // Start the array
    emitter.start();

    // Wait for enter key to be pressed.
    std::cout << "Hit ENTER to quit..." << std::endl;
    std::string line;
    std::getline(std::cin, line);

    // Stop the array
    emitter.stop();

    // Stop asking for data from the Leap controller
    leap_control.removeListener(point.hand);

    return 0;
}
