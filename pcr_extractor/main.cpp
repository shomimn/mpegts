#include <iostream>
#include <fstream>
#include <ts_stream.h>
#include <string>

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
        std::ofstream f(argv[3]);

        stream.on_packet(pid, [&](ts_packet& packet)
        {
            if (packet.has_adaptation_field() && packet.adaptation_field.has_pcr)
            {
                f << packet.adaptation_field.pcr << '\n';
            }
        });

        stream.start();

        f.close();
    }
}