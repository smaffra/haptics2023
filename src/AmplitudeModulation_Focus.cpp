// This example uses the Amplitude Modulation emitter to project a focal point
// at a fixed position above the centre of the array

#include <cstdlib>
#include <iostream>
#include <string>

#include <UltrahapticsAmplitudeModulation.hpp>

int main(int argc, char *argv[])
{
    // Create an emitter.
    Ultrahaptics::AmplitudeModulation::Emitter emitter;

    // Set frequency to 200 Hertz and maximum intensity
    float frequency = 200.0 * Ultrahaptics::Units::hertz;
    float intensity = 1.0f;

    // Position the focal point at 20 centimeters above the array.
    float distance = 20.0 * Ultrahaptics::Units::centimetres;

    // Optionally, specify the focal point distance in cm on the command line
    if (argc > 1)
    {
        distance = atof(argv[1]) * Ultrahaptics::Units::centimetres;
    }

    Ultrahaptics::Vector3 position1(0.0f, 0.0f, distance);
    Ultrahaptics::AmplitudeModulation::ControlPoint point1(position1, intensity, frequency);

    // Emit the point.
    if (!emitter.update(point1))
    {
        std::cout << "Couldn't start emitter." << std::endl;
        return 1;
    }

    // Display the message.
    std::cout << "Hit ENTER to quit...";
    // Wait for enter key to be pressed.
    std::string line;
    std::getline(std::cin, line);

    // Emitter shuts down on exit.
    return 0;
}
