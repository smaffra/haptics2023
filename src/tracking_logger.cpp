#include <iostream>
#include "tracking_logger.hpp"

static const char EVENT_SCHEMA[] =
"{\
		\"type\": \"record\",\
		\"name\": \"Event\",\
		\"fields\" : [\
			{ \"name\": \"time\", \"type\": \"long\" },\
			{ \"name\": \"key\", \"type\": \"int\" },\
			{ \"name\": \"x\", \"type\": \"float\" },\
			{ \"name\": \"y\", \"type\": \"float\" }\
		]\
}";

TrackingEventAvro::TrackingEventAvro()
{
	// creating instance of avro value
	avro_schema_from_json_literal(EVENT_SCHEMA, &schema);
	_iface = avro_generic_class_from_schema(schema);
	avro_generic_value_new(_iface, &value);

	// getting reference to record fields
	avro_value_get_by_index(&value, 0, &_value_time, NULL);
	avro_value_get_by_index(&value, 1, &_value_key, NULL);
	avro_value_get_by_index(&value, 2, &_value_x, NULL);
	avro_value_get_by_index(&value, 3, &_value_y, NULL);
}

TrackingEventAvro::~TrackingEventAvro()
{
	avro_value_iface_decref(_iface);
	avro_schema_decref(schema);
	//avro_generic_value_free(&value);
}

void TrackingEventAvro::update(TrackingEvent& event)
{
	avro_value_set_long(&_value_time, event.time);
	avro_value_set_int(&_value_key, event.key);
	avro_value_set_float(&_value_x, event.x);
	avro_value_set_float(&_value_y, event.y);
}

AvroWriter::AvroWriter()
{
}

AvroWriter::~AvroWriter()
{
}

bool AvroWriter::open(const char* filename, avro_schema_t* schema)
{
	if (avro_file_writer_create_with_codec(filename, *schema, &_writer, "deflate", 0))
	{
		std::cout << "error creating avro file" << std::endl;
		return false;
	}
	return true;
}

bool AvroWriter::append(avro_value_t* value)
{
	avro_file_writer_append_value(_writer, value);
	return true;
}

void AvroWriter::flush()
{
	avro_file_writer_flush(_writer);
}

void AvroWriter::close()
{	
	flush();
	avro_file_writer_close(_writer);
}


TrackingEventLogger::TrackingEventLogger(size_t queue_length) : _events(queue_length)
{
}

TrackingEventLogger::~TrackingEventLogger()
{
}

void TrackingEventLogger::store_event(TrackingEvent& e)
{
	std::lock_guard<std::mutex> guard(_mutex);
	if (!_events.full())
		_events.push(e);
	else
		std::cout << "Queue full, skipping event" << std::endl;
}

void TrackingEventLogger::dump_events(size_t count, bool verbose)
{
	std::lock_guard<std::mutex> guard(_mutex);
	if (verbose)
		std::cout << "dump_events> element count: " << _events.count() << std::endl;
	for (size_t i = 0; i < count; i++)
	{
		if (_events.empty())
			break;
		_event_avro.update(_events.back());
		_writer.append(&_event_avro.value);
		_events.pop();
	}
}

bool TrackingEventLogger::open(const char* filename)
{
	return _writer.open(filename, &_event_avro.schema);
}

void TrackingEventLogger::flush()
{
	_writer.flush();
}

void TrackingEventLogger::close()
{
	_writer.close();
}

#if 0

char* json_str = nullptr;






avro_value_set_int(&value_key, 123);
avro_value_set_float(&value_x, 456.0f);
avro_value_set_float(&value_y, 789.0f);

avro_value_to_json(&record, 0, &json_str);
printf("%s\n", json_str);
free(json_str);



avro_value_set_int(&value_key, 123000);
avro_value_set_float(&value_x, 456000.0f);
avro_value_set_float(&value_y, 789000.0f);

avro_value_to_json(&record, 0, &json_str);
printf("%s\n", json_str);
free(json_str);

avro_file_writer_append_value(writer, &record);

std::cout << " DONE!!!" << std::endl;
return;

#endif
