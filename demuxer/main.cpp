#include <iostream>
#include <ts_stream.h>
#include <string>
#include <thread>

void deleter(FILE* f)
{
	fclose(f);
}

using file_deleter = decltype(&deleter);

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		std::cout << "not enough arguments\n";
		return 0;
	}

	ts_stream stream;
	stream.open(argv[1]);

	if (stream.is_ok())
	{
		int pid = std::stoi(argv[2]);
		std::unique_ptr<FILE, file_deleter> f(fopen(argv[3], "w+b"), deleter);

		if (f)
		{
			stream.on_packet(pid, [&](ts_packet& packet)
			{
				if (packet.has_data())
					fwrite(packet.payload, 1, packet.payload_length, f.get());
			});

			stream.start();
		}
	}
}