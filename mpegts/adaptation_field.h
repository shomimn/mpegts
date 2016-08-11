#pragma once

#include <cstdint>

struct adaptation_field
{
    using flag_ptr = bool adaptation_field::*;
    static constexpr int max_size = 188 - 4;

    uint8_t length;
    bool discontinuity_indicator;
    bool random_access_indicator;
    bool es_priority_indicator;
    bool has_pcr;
    bool has_opcr;
    bool has_splice;
    bool has_private_data;
    bool has_adaptation_ext;
    uint64_t pcr;
    uint64_t opcr;

    flag_ptr flags[8] = {
        &adaptation_field::discontinuity_indicator,
        &adaptation_field::random_access_indicator,
        &adaptation_field::es_priority_indicator,
        &adaptation_field::has_pcr,
        &adaptation_field::has_opcr,
        &adaptation_field::has_splice,
        &adaptation_field::has_private_data,
        &adaptation_field::has_adaptation_ext
    };

    adaptation_field();
    adaptation_field(const adaptation_field& other) = default;
    adaptation_field(adaptation_field&& other) = default;
    adaptation_field& operator=(const adaptation_field& other) = default;
    adaptation_field& operator=(adaptation_field&& other) = default;

    void init(uint8_t* data);
    void init_loop(uint8_t* data);
    void init_unroll(uint8_t* data);

private:
    template<int i>
    void init_unroll_impl(uint8_t* data)
    {
        assign_flag(data, i - 1);
        init_unroll_impl<i - 1>(data);
    }

    template<>
    void init_unroll_impl<1>(uint8_t* data)
    {
        assign_flag(data, 0);
    }

    template<typename F>
    void base_init(uint8_t* data, F&& initializer)
    {
        length = data[4] + 1; //1 for adaptation_field_length

        initializer();

        if (has_pcr)
            calculate_pcr(data);
    }

    void assign_flag(uint8_t* data, int i);
    void calculate_pcr(uint8_t* data);
};