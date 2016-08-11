#pragma once

#include <asio.hpp>
#include <vector>
#include <map>
#include "ts_stream.h"

using asio::ip::udp;
using asio::ip::address;

struct rate_controller
{
	static constexpr int threshold = 512;

	uint64_t pcr;
	int bytes_sent;

	rate_controller()
		: pcr(0)
		, bytes_sent(0)
	{
	}

	void operator()(ts_packet& packet)
	{
		bytes_sent += ts_packet::size;

		if (bytes_sent / 1024 > threshold
			&& packet.has_adaptation_field()
			&& packet.adaptation_field.has_pcr)
		{
			if (pcr == 0)
				pcr = packet.adaptation_field.pcr;
			else
			{
				uint64_t new_pcr = packet.adaptation_field.pcr;
				uint64_t diff = new_pcr - pcr;
				pcr = new_pcr;

				uint64_t delay = diff / 90;
				if (delay > 100)
					delay = 100;

				Sleep(delay);
				//std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			}
		}
	}

	void reset()
	{
		pcr = 0;
		bytes_sent = 0;
	}
};

class udp_streamer
{
private:
	static constexpr int partition_size = 7;
	static constexpr int datagram_size = partition_size * ts_packet::size;

	asio::io_service io_service;
	udp::socket socket;
	udp::resolver resolver;
	udp::endpoint endpoint;
	ts_stream& stream;
	rate_controller rate_control;

	std::array<uint8_t, datagram_size> data;

public:
	udp_streamer();
	udp_streamer(ts_stream& stream);
	udp_streamer(std::string ip_address, short port);
	~udp_streamer();
	udp_streamer(const udp_streamer& other) = delete;
	udp_streamer(udp_streamer&& other) = default;
	udp_streamer& operator=(const udp_streamer& other) = delete;
	udp_streamer& operator=(udp_streamer&& other) = default;

	void set_receiver(std::string& ip_address, std::string& port, udp protocol = udp::v4());
	void set_receiver(std::string&& ip_address, std::string&& port, udp protocol = udp::v4());
	void start(std::string filename);
	void start();
};
