#pragma once

#include <avro.h>
#include <mutex>

#include "tqueue.hpp"
#include "tracking_event.hpp"

class TrackingEventAvro
{
protected:
	avro_value_iface_t* _iface;
	avro_value_t _value_time;
	avro_value_t _value_key;
	avro_value_t _value_x;
	avro_value_t _value_y;

public:
	avro_schema_t schema;
	avro_value_t value;

	TrackingEventAvro();
	virtual ~TrackingEventAvro();
	void update(TrackingEvent& event);	   
};

class AvroWriter
{
protected:
	avro_file_writer_t _writer;

public:
	AvroWriter();
	virtual ~AvroWriter();

	bool open(const char* filename, avro_schema_t* schema);
	bool append(avro_value_t* value);
	void flush();
	void close();
};

class TrackingEventLogger
{
protected:
	std::mutex _mutex;
	CircularQueue<TrackingEvent> _events;
	TrackingEventAvro _event_avro;
	AvroWriter _writer;

public:
	TrackingEventLogger(size_t queue_length);
	virtual ~TrackingEventLogger();
	
	void store_event(TrackingEvent&);
	void dump_events(size_t count, bool verbose);

	bool open(const char* filename);
	void flush();
	void close();
};