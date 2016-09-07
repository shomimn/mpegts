#include "ts_packet.h"
#include "pes_packet.h"

bool invalid_adaptation_length(ts_packet* packet, const int& bytes_read)
{
    return packet->adaptation_field.length > bytes_read - ts_packet::header_size;
}

ts_packet::ts_packet()
    : should_discard(false)
    , pes_start(false)
    , discard_criteria({ &invalid_adaptation_length })
{
}

void ts_packet::init(int bytes_read)
{
    should_discard = false;
    pes_start = false;
    has_pts = false;
    has_dts = false;

    parse_header();
    parse_adaptation_field();

    should_discard = std::any_of(
        discard_criteria.begin(), discard_criteria.end(),
        [&](auto& pred) { return pred(this, bytes_read); });

    if (should_discard)
        return;

    parse_payload(bytes_read);
}

bool ts_packet::has_adaptation_field()
{
    return (adaptation_field_ctrl & 0x20);
}

bool ts_packet::has_data()
{
    return (adaptation_field_ctrl & 0x10);
}

bool ts_packet::is_null()
{
    return pid == null_pid;
}

void ts_packet::parse_header()
{
    sync_byte = data[0];
    transport_error = (data[1] & 0x80);
    payload_start = (data[1] & 0x40);
    transport_priority = (data[1] & 0x20);
    pid = ((uint16_t)(data[1] & 0x1f) << 8) + data[2];
    transport_scrambling_ctrl = (data[3] & 0xc0);
    adaptation_field_ctrl = (data[3] & 0x30);
    continuity_counter = (data[3] & 0x0f);
}

void ts_packet::parse_adaptation_field()
{
    if (has_adaptation_field())
        adaptation_field.init(&data[0]);
    //adaptation_field.init_loop(&data[0]);
    //adaptation_field.init_unroll(&data[0]);
    else
        adaptation_field.length = 0;
}

void ts_packet::parse_payload(const int& bytes_read)
{
    payload_length = bytes_read - header_size - adaptation_field.length;
    int start = header_size + adaptation_field.length;

    if (has_data())
        payload = &data[start];

    if (payload_start)
    {
        uint32_t start_code = data[start] << 16;
        start_code |= data[start + 1] << 8;
        start_code |= data[start + 2];

        if (start_code == pes_packet::start_code)
        {
            pes_start = true;

            pes_packet_length = data[start + 4] << 8;
            pes_packet_length |= data[start + 5];
            pes_packet_length += pes_packet::header_size; //6 bytes before and including the length field

            has_pts = data[start + 7] & 0x80;
            has_dts = has_pts && data[start + 7] & 0x40;

            pes_header_length = data[start + 8];

            int payload_offset = pes_packet::header_size + pes_packet::optional_header_size + pes_header_length;

            payload += payload_offset;
            payload_length -= payload_offset;

            if (has_pts)
                pts = parse_timestamp(&data[start + 9]);

            if (has_dts)
                dts = parse_timestamp(&data[start + 14]);
        }
    }
}

uint64_t ts_packet::parse_timestamp(uint8_t* from)
{
    uint64_t ts = (from[0] & 0x0e) << 29;
    ts |= from[1] << 22;
    ts |= (from[2] & 0xfe) << 14;   
    ts |= from[3] << 7;
    ts |= from[4] >> 1;

    return ts;
}
