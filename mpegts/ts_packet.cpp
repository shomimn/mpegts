#include "ts_packet.h"
#include "pes_packet.h"

ts_packet::ts_packet()
    : should_discard(false)
{
}

void ts_packet::init(int bytes_read)
{
    should_discard = false;

    parse_header();
    parse_adaptation_field();

    should_discard |= [&]() { return adaptation_field.length > bytes_read - header_size; }();

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
            pes_packet_length = data[start + 4] << 8;
            pes_packet_length |= data[start + 5];
            pes_packet_length += pes_packet::header_size; //6 bytes before and including the length field

            pes_header_length = data[start + 8];

            int payload_offset = pes_packet::header_size + pes_packet::optional_header_size + pes_header_length;

            payload += payload_offset;
            payload_length -= payload_offset;
        }
    }
}
