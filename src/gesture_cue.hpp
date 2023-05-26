#pragma once

#include <cstdio>
#include <string>
#include <vector>
#include <Ultrahaptics.hpp>

class GestureCue
{
protected:
	size_t _current_time_point;
	size_t _num_time_points;
	size_t _num_control_points;

public:
	std::vector<std::vector<float>> intensity;
	std::vector<std::vector<Ultrahaptics::Vector3>> position;

	GestureCue() : _current_time_point(0), _num_control_points(0), _num_time_points(0)
	{
	}
	
	virtual ~GestureCue()
	{
	}

	void reset_time_point()
	{
		_current_time_point = 0;
	}

	size_t next_time_point()
	{
		_current_time_point = (_current_time_point + 1) % _num_time_points;
		return _current_time_point;
	}

	size_t current_time_point()
	{
		return _current_time_point;
	}

	size_t num_control_points()
	{
		return _num_control_points;
	}

	size_t num_time_points()
	{
		return _num_time_points;
	}

	Ultrahaptics::Vector3& current_position(size_t control_point)
	{
		return position[_current_time_point][control_point];
	}

	float current_intensity(size_t control_point)
	{
		return intensity[_current_time_point][control_point];
	}
	
	bool load(const std::string& filename)
	{
		FILE* f = fopen(filename.c_str(), "rt" );
		
		if(f == NULL)
			return false;
		
		int npoints;
		int nrow;
		int ncol;
		float x, y, z, w;
		
		fscanf(f, "%d", &npoints);
		fscanf(f, "%d", &nrow);
		fscanf(f, "%d", &ncol);

		_num_time_points = (size_t) nrow;
		_num_control_points = (size_t) npoints;

		intensity.resize(nrow, std::vector<float>(_num_control_points));
		position.resize(nrow, std::vector<Ultrahaptics::Vector3>(_num_control_points));
		
		for(size_t t = 0; t < _num_time_points; t++)
		{
			for (size_t p = 0; p < _num_control_points; p++)
			{
				fscanf(f, "%f", &x);
				fscanf(f, "%f", &y);
				fscanf(f, "%f", &z);
				fscanf(f, "%f", &w);
				position[t][p].x = x;
				position[t][p].y = y;
				position[t][p].z = z;
				intensity[t][p] = w;
			}
		}
		fclose(f);
		return true;
	}	
};

