#pragma once

#include <cstdint>
#include <array>
#include "adaptation_field.h";

struct ts_packet
{
	static constexpr int size = 188;
	static constexpr int header_size = 4;
	static constexpr int null_pid = 0x1fff;

	std::array<uint8_t, size> data;
	uint8_t sync_byte;
	bool transport_error;
	bool payload_start;
	bool transport_priority;
	uint16_t pid;
	uint8_t transport_scrambling_ctrl;
	uint8_t adaptation_field_ctrl;
	uint8_t continuity_counter;
	adaptation_field adaptation_field;
	uint8_t* payload;
	uint8_t payload_length;
	uint16_t pes_packet_length;
	uint8_t pes_header_length;
	bool should_discard;

	ts_packet();
	ts_packet(const ts_packet& other) = default;
	ts_packet(ts_packet&& other) = default;
	ts_packet& operator=(const ts_packet& other) = default;
	ts_packet& operator=(ts_packet&& other) = default;

	void init(int bytes_read);
	bool has_adaptation_field();
	bool has_data();
	bool is_null();

private:
	void parse_header();
	void parse_adaptation_field();
	void parse_payload(const int& bytes_read);
};
