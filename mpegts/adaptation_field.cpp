#include "adaptation_field.h"

adaptation_field::adaptation_field()
    : length(0)
{
}

void adaptation_field::init(uint8_t* data)
{
	base_init(data, [&]()
	{
		discontinuity_indicator = (data[5] & 0x80);
		random_access_indicator = (data[5] & 0x40);
		es_priority_indicator = (data[5] & 0x20);
		has_pcr = (data[5] & 0x10);
		has_opcr = (data[5] & 0x08);
		has_splice = (data[5] & 0x04);
		has_private_data = (data[5] & 0x02);
		has_adaptation_ext = (data[5] & 0x01);
	});
}

void adaptation_field::init_loop(uint8_t* data)
{
	base_init(data, [&]()
	{
		for (int i = 0; i < 8; ++i)
			assign_flag(data, i);
	});
}

void adaptation_field::init_unroll(uint8_t* data)
{
	base_init(data, [&]()
	{
		init_unroll_impl<8>(data);
	});
}

void adaptation_field::assign_flag(uint8_t* data, int i)
{
    this->*flags[i] = (data[5] & (0x80 >> i));
}

void adaptation_field::calculate_pcr(uint8_t* data)
{
    pcr = data[6] << 25;
    pcr |= data[7] << 17;
    pcr |= data[8] << 9;
    pcr |= data[9] << 1;
    pcr |= data[10] >> 7;

    //pcr *= 300;
    //calculate and add extension if necessary
}