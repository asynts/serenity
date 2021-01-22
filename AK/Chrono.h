/*
 * Copyright (c) 2021, Paul Scharnofske
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <AK/Format.h>
#include <AK/Types.h>

namespace Chrono {

struct ClockRatio {
    u64 nominator;
    u64 denominator;

    static constexpr ClockRatio hours() { return { 60 * 60, 1 }; }
    static constexpr ClockRatio minutes() { return { 60, 1 }; }
    static constexpr ClockRatio seconds() { return { 1, 1 }; }
    static constexpr ClockRatio milliseconds() { return { 1, 1'000 }; }
    static constexpr ClockRatio microseconds() { return { 1, 1'000'000 }; }
    static constexpr ClockRatio nanoseconds() { return { 1, 1'000'000'000 }; }
};

template<ClockRatio Ratio>
class Duration {
public:
    explicit constexpr Duration(i64 ticks = 0)
        : m_ticks(ticks)
    {
    }

    static constexpr Duration zero() { return { 0 }; }

    constexpr i64 ticks() const { return m_ticks; }
    constexpr ClockRatio ratio() const { return Ratio; }

    constexpr Duration<ClockRatio::hours()> in_hours() const { return cast<ClockRatio::hours()>(); }
    constexpr Duration<ClockRatio::minutes()> in_minutes() const { return cast<ClockRatio::minutes()>(); }
    constexpr Duration<ClockRatio::seconds()> in_seconds() const { return cast<ClockRatio::seconds()>(); }
    constexpr Duration<ClockRatio::milliseconds()> in_milliseconds() const { return cast<ClockRatio::milliseconds()>(); }
    constexpr Duration<ClockRatio::microseconds()> in_microseconds() const { return cast<ClockRatio::microseconds()>(); }
    constexpr Duration<ClockRatio::nanoseconds()> in_nanoseconds() const { return cast<ClockRatio::nanoseconds()>(); }

    template<ClockRatio Ratio2>
    constexpr Duration<Ratio2> cast() const
    {
        // FIXME: Be careful with overflowing u64.
        // FIXME: Be careful with floating point accuracy.
        return { (ticks() * Ratio.denominator * Ratio2.nominator) / (Ratio.nominator * Ratio2.denominator) };
    }

    constexpr Duration operator+(Duration rhs) const { return { ticks() + rhs.ticks() }; }
    constexpr Duration operator-(Duration rhs) const { return { ticks() - rhs.ticks() }; }

    constexpr Duration& operator+=(Duration rhs) const { return *this = *this + rhs; }
    constexpr Duration& operator-=(Duration rhs) const { return *this = *this - rhs; }

    constexpr bool operator<=(Duration rhs) const { return (*this - rhs).ticks() <= 0; }
    constexpr bool operator>=(Duration rhs) const { return (*this - rhs).ticks() >= 0; }
    constexpr bool operator<(Duration rhs) const { return (*this - rhs).ticks() < 0; }
    constexpr bool operator>(Duration rhs) const { return (*this - rhs).ticks() > 0; }
    constexpr bool operator==(Duration rhs) const { return (*this - rhs).ticks() == 0; }

private:
    i64 m_ticks;
};

using Hours = Duration<ClockRatio::hours()>;
using Minutes = Duration<ClockRatio::minutes()>;
using Seconds = Duration<ClockRatio::seconds()>;
using Milliseconds = Duration<ClockRatio::milliseconds()>;
using Microseconds = Duration<ClockRatio::microseconds()>;
using Nanoseconds = Duration<ClockRatio::nanoseconds()>;

template<ClockRatio Ratio>
class Instant {
public:
    explicit constexpr Instant(u64 ticks = 0)
        : m_ticks(ticks)
    {
    }

    static constexpr Instant epoch() { return { 0 }; }

    constexpr ClockRatio ratio() const { return Ratio; }
    constexpr u64 ticks() const { return m_ticks; }

    template<ClockRatio Ratio2>
    constexpr Instant<Ratio2> cast()
    {
        // FIXME: Be careful with overflowing u64.
        // FIXME: Be careful with floating point accuracy.
        return { (ticks() * Ratio.denominator * Ratio2.nominator) / (Ratio.nominator * Ratio2.denominator) };
    }

    constexpr Duration<Ratio> operator-(Instant rhs) const
    {
        return { static_cast<i64>(ticks()) - static_cast<i64>(rhs.ticks()) };
    }

    constexpr Instant operator+(Duration<Ratio> rhs) const { return { ticks() + rhs.ticks() }; }
    constexpr Instant operator-(Duration<Ratio> rhs) const { return { ticks() - rhs.ticks() }; }

    constexpr Instant& operator+=(Duration<Ratio> rhs) { return *this = *this + rhs; }
    constexpr Instant& operator-=(Duration<Ratio> rhs) { return *this = *this - rhs; }

    constexpr bool operator<=(Instant rhs) const { return (*this - rhs).ticks() <= Duration<Ratio>::zero(); }
    constexpr bool operator>=(Instant rhs) const { return (*this - rhs).ticks() >= Duration<Ratio>::zero(); }
    constexpr bool operator<(Instant rhs) const { return (*this - rhs).ticks() < Duration<Ratio>::zero(); }
    constexpr bool operator>(Instant rhs) const { return (*this - rhs).ticks() > Duration<Ratio>::zero(); }
    constexpr bool operator==(Instant rhs) const { return (*this - rhs).ticks() == Duration<Ratio>::zero(); }

private:
    u64 m_ticks;
};

struct MonotonicClock {
public:
    static Nanoseconds now();
};

struct RealtimeClock {
public:
    static Nanoseconds now();
};

class SplittedDuration {
public:
    explicit constexpr SplittedDuration(Nanoseconds value)
    {
        m_hours = value.in_hours();
        value -= m_hours.in_nanoseconds();

        m_minutes = value.in_minutes();
        value -= m_minutes.in_nanoseconds();

        m_seconds = value.in_seconds();
        value -= m_seconds.in_nanoseconds();

        m_milliseconds = value.in_milliseconds();
        value -= m_milliseconds.in_nanoseconds();

        m_microseconds = value.in_microseconds();
        value -= m_microseconds.in_nanoseconds();

        m_nanoseconds = value.in_nanoseconds();
        value -= m_nanoseconds;

        ASSERT(value == 0ns);
    }

    constexpr Chrono::Hours hours() const { return m_hours; }
    constexpr Chrono::Minutes minutes() const { return m_minutes; }
    constexpr Chrono::Seconds seconds() const { return m_seconds; }
    constexpr Chrono::Milliseconds milliseconds() const { return m_milliseconds; }
    constexpr Chrono::Microseconds microseconds() const { return m_microseconds; }
    constexpr Chrono::Nanoseconds nanoseconds() const { return m_nanoseconds; }

private:
    Chrono::Hours m_hours;
    Chrono::Minutes m_minutes;
    Chrono::Seconds m_seconds;
    Chrono::Milliseconds m_milliseconds;
    Chrono::Microseconds m_microseconds;
    Chrono::Nanoseconds m_nanoseconds;
};

}

Chrono::Hours operator""h(u64 value) { return Chrono::Hours { value }; }
Chrono::Minutes operator"" m(u64 value) { return Chrono::Minutes { value }; }
Chrono::Seconds operator""s(u64 value) { return Chrono::Seconds { value }; }
Chrono::Milliseconds operator""ms(u64 value) { return Chrono::Milliseconds { value }; }
Chrono::Microseconds operator""us(u64 value) { return Chrono::Microseconds { value }; }
Chrono::Nanoseconds operator""ns(u64 value) { return Chrono::Nanoseconds { value }; }

template<Chrono::ClockRatio Ratio>
struct AK::Formatter<Chrono::Duration<Ratio>> : Formatter<Chrono::Nanoseconds> {
    void format(FormatBuilder& builder, Chrono::Duration<Ratio> value)
    {
        return Formatter<Chrono::Nanoseconds>(builder, value.in_nanoseconds());
    }
};
template<>
struct AK::Formatter<Chrono::Nanoseconds> {
    void parse(TypeErasedFormatParams&, FormatParser& parser)
    {
        // FIXME: Parse fill-and-align, width and precision.

        if (parser.is_eof())
            m_specifier = "%H:%M:%S.%n";
        else
            m_specifier = parser.consume_all();
    }

    void format(FormatBuilder&, Chrono::Nanoseconds);

    StringView m_specifier;
};
