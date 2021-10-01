/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <chrono>

namespace util
{
template<typename rt, typename dt>
static inline const rt seconds(const dt &duration)
{
    return static_cast<rt>(static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(duration).count()) / 1000000.0f);
}
} // namespace util

template<typename T>
class ChronoClock final {
public:
    ChronoClock();
    const typename T::duration elapsed() const;
    const typename T::duration restart();
    static const typename T::time_point now();

private:
    typename T::time_point start;
};

template<typename T>
inline ChronoClock<T>::ChronoClock()
    : start(now())
{

}

template<typename T>
inline const typename T::duration ChronoClock<T>::elapsed() const
{
    return now() - start;
}

template<typename T>
inline const typename T::duration ChronoClock<T>::restart()
{
    const typename T::time_point new_start = now();
    const typename T::duration delta = (new_start - start);
    start = new_start;
    return delta;
}

template<typename T>
inline const typename T::time_point ChronoClock<T>::now()
{
    return T::now();
}
