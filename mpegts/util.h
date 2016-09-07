#pragma once
#include <type_traits>

template<typename T, typename U, typename... Ts>
struct all_same : std::false_type {};

template<typename T>
struct all_same<T, T> : std::true_type {};

template<typename T, typename... Ts>
struct all_same<T, T, Ts...> : all_same<T, Ts...> {};

template<typename T>
using raw = std::remove_reference_t<std::remove_cv_t<T>>;

template<typename... Bools>
void reset_flags(Bools&&... b)
{
    static_assert(
        all_same<bool, raw<Bools>...>::value,
        "arguments to reset_flags have to be bools");

    std::initializer_list<int> { (b = false, 0)... };
}