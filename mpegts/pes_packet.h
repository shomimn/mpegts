#pragma once

#include <vector>
#include <cstdint>

struct pes_packet
{
	static constexpr int start_code = 0x000001;
	static constexpr int header_size = 6;
	static constexpr int optional_header_size = 3;

    uint16_t length;
    std::vector<uint8_t> data;

    pes_packet(uint16_t length);
    pes_packet(const pes_packet& other) = default;
    pes_packet(pes_packet&& other) = default;
    pes_packet& operator=(const pes_packet& other) = default;
    pes_packet& operator= (pes_packet&& other) = default;
};
