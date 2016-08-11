#include "pes_packet.h"

pes_packet::pes_packet(uint16_t length)
	: length(length)
{
	data.reserve(length);
}
