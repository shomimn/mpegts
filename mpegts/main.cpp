#include <iostream>
#include <vector>
#include <iterator>
#include "ts_packet.h"
#include "ts_stream.h"
#include <thread>
#include <sstream>

#include <Windows.h>

namespace command
{
    << << << < HEAD
        const std::string open = "open";
    const std::string stream = "stream";
    const std::string demux = "demux";
    const std::string pcr = "pcr";
    const std::string metadata = "metadata";
    const std::string exit = "exit";
    == == == =
        const std::string open = "open";
    const std::string stream = "stream";
    const std::string demux = "demux";
    const std::string pcr = "pcr";
    const std::string metadata = "metadata";
    const std::string exit = "exit";
    >> >> >> > 6fbd6c2d19ee407e9880bf7999c1a923e97ef79b
}

std::vector<std::string> whitespace_split(std::string& line)
{
    std::stringstream ss(line);

    return std::vector<std::string>(
        std::istream_iterator<std::string>(ss),
        std::istream_iterator<std::string>());
}

bool start_process(std::string process, std::string filename, std::string arg1, std::string arg2)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    std::string path = "C:\\Users\\Milos\\Documents\\Visual Studio 2015\\Projects\\libdvbpsi\\Release\\" + process;
    std::string cmd = process + " " + filename + " " + arg1 + " " + arg2;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // start the program up
    bool ok = CreateProcess(
        const_cast<wchar_t*>(std::wstring(path.begin(), path.end()).c_str()),   // Path
        const_cast<wchar_t*>(std::wstring(cmd.begin(), cmd.end()).c_str()),     // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,              // Creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi);           // Pointer to PROCESS_INFORMATION structure

    return ok;
}

int main(int argc, char* argv[])
{
    std::string line;
    std::string start("> ");
    std::vector<std::string> tokens;
    ts_stream stream;

    while (line != command::exit)
    {
        std::cout << start;
        std::getline(std::cin, line);

        tokens = whitespace_split(line);

        if (tokens[0] == command::demux)
            start_process("demuxer.exe", tokens[1], tokens[2], tokens[3]);
        else if (tokens[0] == command::stream)
            start_process("streamer.exe", tokens[1], tokens[2], tokens[3]);
        else if (tokens[0] == command::pcr)
            start_process("pcr_extractor.exe", tokens[1], tokens[2], tokens[3]);
        else if (tokens[0] == command::metadata)
        {
            ts_stream stream;
            stream.open(tokens[1]);

            if (stream.is_ok())
                stream.show_metadata();
            else
                std::cout << "can't open stream\n";
        }
    }
}