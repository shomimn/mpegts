#include <iostream>
#include <ts_stream.h>
#include <udp_streamer.h>
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
        udp_streamer streamer(stream);
        streamer.set_receiver(argv[2], argv[3]);
        streamer.start();
    }
}