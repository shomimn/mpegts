#pragma once

#include <functional>
#include <cstdint>
#include <memory>
#include <map>
#include <vector>
#include "ts_packet.h"

extern "C"
{
#include <dvbpsi.h>
#include <psi.h>
#include <tables\pat.h> 
#include <descriptor.h>
#include <tables\pmt.h>
#include <descriptors\dr.h>
}

class ts_stream;
void dump_pat(void* data, dvbpsi_pat_t* pat);
void dump_pmt(void* data, dvbpsi_pmt_t* pmt);

struct pat_t
{
    dvbpsi_t* handle;
    bool attached;

    pat_t(ts_stream* stream)
        : handle(nullptr)
        , attached(false)
    {
        handle = dvbpsi_new(NULL, DVBPSI_MSG_NONE);

        if (handle)
            attached = dvbpsi_pat_attach(handle, dump_pat, stream);
    }

    ~pat_t()
    {
        if (attached)
            dvbpsi_pat_detach(handle);

        if (handle)
            dvbpsi_delete(handle);
    }

    pat_t(const pat_t& other) = delete;
    pat_t(pat_t&& other)
        : handle(std::move(other.handle))
        , attached(std::move(other.attached))
    {
        other.handle = nullptr;
        other.attached = false;
    }

    pat_t& operator=(const pat_t& other) = delete;
    pat_t& operator=(pat_t&& other)
    {
        handle = std::move(other.handle);
        attached = std::move(other.attached);

        other.handle = nullptr;
        other.attached = false;

        return *this;
    }
};

struct pmt_t
{
    dvbpsi_t* handle = nullptr;
    bool attached = false;

    pmt_t()
        : handle(nullptr)
        , attached(false)
    {
    }

    pmt_t(ts_stream* stream, int program_number)
        : pmt_t()
    {
        handle = dvbpsi_new(NULL, DVBPSI_MSG_NONE);

        if (handle)
            attached = dvbpsi_pmt_attach(handle, program_number, dump_pmt, stream);
    }

    ~pmt_t()
    {
        if (attached)
            dvbpsi_pmt_detach(handle);

        if (handle)
            dvbpsi_delete(handle);
    }

    pmt_t(const pmt_t& other) = delete;
    pmt_t(pmt_t&& other)
        : handle(std::move(other.handle))
        , attached(std::move(other.attached))
    {
        other.handle = nullptr;
        other.attached = false;
    }

    pmt_t& operator=(const pmt_t& other) = delete;
    pmt_t& operator=(pmt_t&& other)
    {
        handle = std::move(other.handle);
        attached = std::move(other.attached);

        other.handle = nullptr;
        other.attached = false;

        return *this;
    }
};

class ts_stream
{
private:
    using packet_fn = std::function<void(ts_packet&)>;
    using pes_fn = std::function<void(std::vector<uint8_t>&)>;
    using nullary_fn = std::function<void()>;

    using pes_data = std::tuple<bool, std::vector<uint8_t>, pes_fn>;

    int fd;
    pat_t pat;
    std::map<int, pmt_t> pmt_tables;

    std::map<int, packet_fn> packet_handlers;
    std::map<int, pes_data> pes_handlers;
    packet_fn any_packet_handler;
    nullary_fn start_handler;
    nullary_fn end_handler;

    int read(ts_packet& packet);
    void collect(ts_packet& packet);

public:
    ts_stream();
    ~ts_stream();
    ts_stream(const ts_stream& other) = delete;
    ts_stream(ts_stream&& other) = delete;
    ts_stream& operator=(const ts_stream& other) = delete;
    ts_stream& operator=(ts_stream&& other) = delete;

    void on_packet(int pid, packet_fn& f);
    void on_packet(int pid, packet_fn&& f);
    void on_pes(int pid, pes_fn& f);
    void on_pes(int pid, pes_fn&& f);
    void on_any_packet(packet_fn& f);
    void on_any_packet(packet_fn&& f);
    void on_start(nullary_fn& f);
    void on_start(nullary_fn&& f);
    void on_end(nullary_fn& f);
    void on_end(nullary_fn&& f);
    void open(std::string filename);
    bool is_ok();
    void start();
    void reset();
    void show_metadata();

    friend void dump_pat(void* data, dvbpsi_pat_t* pat);
    friend void dump_pmt(void* data, dvbpsi_pmt_t* pmt);
};
