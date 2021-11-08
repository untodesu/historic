/*
 * cvar_numeric.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/math/math.hpp>
#include <common/traits.hpp>
#include <limits>
#include <shared/script/cvar_base.hpp>
#include <sstream>

template<typename T>
class CVarNumeric final : public CVarBase {
    static_assert(std::is_arithmetic_v<T>);

public:
    CVarNumeric(const std::string &name, const T default_value, const fcvar_t flags = 0, const T min = std::numeric_limits<T>::min(), const T max = std::numeric_limits<T>::max());

    inline constexpr const T getValue() const
    {
        return value;
    }

    inline constexpr const T getMin() const
    {
        return min;
    }

    inline constexpr const T getMax() const
    {
        return max;
    }

protected:
    void touch(const std::string &value) override;

private:
    T value;
    const T min, max;
};

template<typename T>
CVarNumeric<T>::CVarNumeric(const std::string &name, const T default_value, const fcvar_t flags, const T min, const T max)
    : CVarBase(name, std::to_string(default_value), flags), value(default_value), min(min), max(max)
{

}

template<typename T>
void CVarNumeric<T>::touch(const std::string &value)
{
    CVarBase::touch(value);
    std::stringstream(value) >> this->value;
    this->value = math::clamp<T>(this->value, min, max);
}
