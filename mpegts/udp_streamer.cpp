#include "udp_streamer.h"

udp_streamer::udp_streamer()
	: socket(io_service, udp::endpoint(udp::v4(), 12346))
	, resolver(io_service)
	, stream(ts_stream())
{
}

udp_streamer::udp_streamer(ts_stream& stream)
	: socket(io_service, udp::endpoint(udp::v4(), 12346))
	, resolver(io_service)
	, stream(stream)
{
}

udp_streamer::udp_streamer(std::string ip_address, short port)
	: socket(io_service, udp::endpoint(address::from_string(ip_address), port))
	, resolver(io_service)
	, stream(stream)
{
}

udp_streamer::~udp_streamer()
{
	socket.shutdown(udp::socket::shutdown_both);
	socket.close();
}

void udp_streamer::set_receiver(std::string& ip_address, std::string& port, udp protocol)
{
	udp::resolver::query query(protocol, ip_address, port);
	endpoint = *resolver.resolve(query);
}

void udp_streamer::set_receiver(std::string&& ip_address, std::string&& port, udp protocol)
{
	udp::resolver::query query(std::move(protocol),
		std::move(ip_address), std::move(port));

	endpoint = *resolver.resolve(query);
}

void udp_streamer::start(std::string filename)
{
	stream.open(filename);
	start();
}

void udp_streamer::start()
{
	if (stream.is_ok())
	{
		int packet_counter = 0;
		auto buffer = asio::buffer(data, datagram_size);

		stream.on_start([&]()
		{
			rate_control.reset();
		});

		stream.on_any_packet([&](ts_packet& packet)
		{
			int start = packet_counter * ts_packet::size;

			std::copy(packet.data.begin(), packet.data.end(), data.begin() + start);
			++packet_counter;

			rate_control(packet);

			if (packet_counter % partition_size == 0)
			{
				packet_counter = 0;

				//s.send_to(buffer, receiver_endpoint);
				socket.async_send_to(buffer, endpoint,
					[](auto& error_code, auto& bytes) {});
			}
		});

		while (true)
		{
			stream.start();

			if (packet_counter != 0)
			{
				socket.send_to(
					asio::buffer(data, packet_counter * ts_packet::size),
					endpoint);
			}

			stream.reset();
		}
	}
}
