/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <chrono>

namespace util
{
template<typename rtype, typename duration_type>
static inline const rtype seconds(const duration_type &duration)
{
    return static_cast<rtype>(static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(duration).count()) / 1000000.0);
}

template<typename clock_type>
class Clock final {
public:
    Clock();
    const typename clock_type::duration elapsed() const;
    const typename clock_type::duration reset();
    static const typename clock_type::time_point now();

private:
    typename clock_type::time_point start_time;
};
} // namespace util

template<typename clock_type>
inline util::Clock<clock_type>::Clock()
    : start_time(now)
{

}

template<typename clock_type>
inline const typename clock_type::duration util::Clock<clock_type>::elapsed() const
{
    return now() - start;
}

template<typename clock_type>
inline const typename clock_type::duration util::Clock<clock_type>::reset() const
{
    const typename clock_type::time_point new_start = now();
    const typename clock_type::duration delta = new_start - start;
    start = new_start;
    return delta;
}

template<typename clock_type>
inline const typename clock_type::time_point Clock<clock_type>::now()
{
    return clock_type::now();
}
