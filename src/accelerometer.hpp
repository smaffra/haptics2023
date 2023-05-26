#pragma once

#include <chrono>
#include <cmath>

class Accelerometer
{
protected:
    float _lastx;
    float _lasty;
    float _lastz;
    float _lastv;
    float _lasta;
    std::chrono::high_resolution_clock::time_point _lastt;

public:

    Accelerometer() //: _lastx(0.0f), _lasty(0.0f), _lastz(0.0f), _lastv(0.0f), _lasta(0.0f)
    {
        _lastx = std::nanf("");
        _lasty = std::nanf("");
        _lastz = std::nanf("");
        _lastv = std::nanf("");
        _lasta = std::nanf("");
    }

    void push(float x, float y, float z, std::chrono::high_resolution_clock::time_point t)    
    {
        float dx = x - _lastx;
        float dy = y - _lasty;
        float dz = z - _lastz;
        float ds = sqrt(dx*dx + dy*dy + dz*dz);

        std::chrono::duration<float> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t - _lastt);
        float dt = time_span.count();

        float v = ds / dt;
        
        _lasta = (v - _lastv) / dt;
        _lastv = v;
        _lastx = x;
        _lasty = y;
        _lastz = z;
        _lastt = t;
    }

    float x(void) { return _lastx; }
    float y(void) { return _lastx; }
    float z(void) { return _lastx; }
    float velocity(void) { return _lastv; }
    float acceleration(void) { return _lasta; }
};
