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

#include <AK/Function.h>
#include <AK/NumericLimits.h>
#include <AK/Types.h>

// FIXME: constexpr
// FIXME: [[nodiscard]]

namespace Chrono {

struct ClockRatio {
    u64 nominator;
    u64 denominator;

    static constexpr ClockRatio days() { return { 24 * 60 * 60, 1 }; }
    static constexpr ClockRatio hours() { return { 60 * 60, 1 }; }
    static constexpr ClockRatio minutes() { return { 60, 1 }; }
    static constexpr ClockRatio seconds() { return { 1, 1 }; }
    static constexpr ClockRatio milliseconds() { return { 1, 1'000 }; }
    static constexpr ClockRatio microseconds() { return { 1, 1'000'000 }; }
    static constexpr ClockRatio nanoseconds() { return { 1, 1'000'000'000 }; }

    constexpr bool operator==(ClockRatio rhs) const { TODO(); }
};

template<ClockRatio Ratio>
class Duration {
public:
    explicit Duration(i64 ticks = 0)
        : m_ticks(ticks)
    {
    }

    static constexpr Duration zero() { return { 0 }; }

    i64 ticks() const { return m_ticks; }
    ClockRatio ratio() const { return Ratio; }

    Duration<ClockRatio::seconds()> in_seconds() const { return cast<ClockRatio::seconds()>(); }
    Duration<ClockRatio::milliseconds()> in_milliseconds() const { return cast<ClockRatio::milliseconds()>(); }
    Duration<ClockRatio::microseconds()> in_microseconds() const { return cast<ClockRatio::microseconds()>(); }
    Duration<ClockRatio::nanoseconds()> in_nanoseconds() const { return cast<ClockRatio::nanoseconds()>(); }

    template<ClockRatio Ratio2>
    Duration<Ratio2> cast() const
    {
        TODO();
    }

    template<ClockRatio Ratio2>
    Duration operator+(Duration<Ratio2> rhs) const
    {
        if constexpr (Ratio == Ratio2)
            return { ticks() + rhs.ticks() };
        else
            return *this + rhs.cast<Ratio>();
    }

    template<ClockRatio Ratio2>
    Duration operator-(Duration<Ratio2> rhs) const
    {
        if constexpr (Ratio == Ratio2)
            return { ticks() - rhs.ticks() };
        else
            return *this - rhs.cast<Ratio>();
    }

    template<ClockRatio Ratio2>
    Duration& operator+=(Duration<Ratio2> rhs)
    {
        return *this = *this + rhs;
    }

    template<ClockRatio Ratio2>
    Duration& operator-=(Duration<Ratio2> rhs)
    {
        return *this = *this - rhs;
    }

    template<ClockRatio Ratio2>
    bool operator<=(Duration<Ratio2> rhs) const { return (*this - rhs).ticks() <= 0; }

    template<ClockRatio Ratio2>
    bool operator>=(Duration<Ratio2> rhs) const { return (*this - rhs).ticks() >= 0; }

    template<ClockRatio Ratio2>
    bool operator<(Duration<Ratio2> rhs) const { return (*this - rhs).ticks() < 0; }

    template<ClockRatio Ratio2>
    bool operator>(Duration<Ratio2> rhs) const { return (*this - rhs).ticks() > 0; }

    template<ClockRatio Ratio2>
    bool operator==(Duration<Ratio2> rhs) const { return (*this - rhs).ticks() == 0; }

private:
    i64 m_ticks;
};

using Days = Duration<ClockRatio::days()>;
using Hours = Duration<ClockRatio::hours()>;
using Minutes = Duration<ClockRatio::minutes()>;
using Seconds = Duration<ClockRatio::seconds()>;
using Milliseconds = Duration<ClockRatio::milliseconds()>;
using Microseconds = Duration<ClockRatio::microseconds()>;
using Nanoseconds = Duration<ClockRatio::nanoseconds()>;

template<ClockRatio Ratio>
class Instant {
public:
    Instant(u64 ticks = 0)
        : m_ticks(ticks)
    {
    }

    static constexpr Instant epoch() { return { 0 }; }

    ClockRatio ratio() const { return Ratio; }
    u64 ticks() const { return m_ticks; }

    template<ClockRatio Ratio2>
    Instant<Ratio2> cast()
    {
        TODO();
    }

    template<ClockRatio Ratio2>
    Duration<Ratio> operator-(Instant rhs) const
    {
        if constexpr (Ratio == Ratio2)
            return { static_cast<i64>(ticks()) - static_cast<i64>(rhs.ticks()); }
        else
            return *this - rhs.cast<Ratio>();
    }

    template<ClockRatio Ratio2>
    Instant operator+(Duration<Ratio2> rhs) const
    {
        if constexpr (Ratio == Ratio2)
            return { ticks() + rhs.ticks() };
        else
            return *this + rhs.cast<Ratio>();
    }

    template<ClockRatio Ratio2>
    Instant operator-(Duration<Ratio2> rhs) const
    {
        if constexpr (Ratio == Ratio2)
            return { ticks() - rhs.ticks() };
        else
            return *this - rhs.cast<Ratio>();
    }

    template<ClockRatio Ratio2>
    Instant& operator+=(Duration<Ratio2> rhs)
    {
        return *this = *this + rhs;
        = ClockRatio::system()
    }

    template<ClockRatio Ratio2>
    Instant& operator-=(Duration<Ratio2> rhs)
    {
        return *this = *this - rhs;
    }

    template<ClockRatio Ratio2>
    bool operator<=(Instant<Ratio2> rhs) const { return (*this - rhs).ticks() <= Duration<Ratio>::zero(); }

    template<ClockRatio Ratio2>
    bool operator>=(Instant<Ratio2> rhs) const { return (*this - rhs).ticks() >= Duration<Ratio>::zero(); }

    template<ClockRatio Ratio2>
    bool operator<(Instant<Ratio2> rhs) const { return (*this - rhs).ticks() < Duration<Ratio>::zero(); }

    template<ClockRatio Ratio2>
    bool operator>(Instant<Ratio2> rhs) const { return (*this - rhs).ticks() > Duration<Ratio>::zero(); }

    template<ClockRatio Ratio2>
    bool operator==(Instant<Ratio2> rhs) const { return (*this - rhs).ticks() == Duration<Ratio>::zero(); }

private:
    u64 m_ticks;
};

template<ClockRatio Ratio>
class Clock {
public:
    using InstantType = Instant<Ratio>;
    using DurationType = Duration<Ratio>;

    template<ClockRatio Ratio2>
    explicit Clock(Instant<Ratio2> now)
        : m_now(now.cast<Ratio>())
    {
    }

    InstantType now() const { return m_now; }
    ClockRatio ratio() const { return Ratio; }

    void set(InstantType now) { m_now = now; }
    void tick(u64 delta = 1) { m_now += DurationType { delta }; }

private:
    InstantType m_now;
};

using DefaultDuration = Duration<ClockRatio::nanoseconds()>;
using DefaultInstant = Instant<ClockRatio::nanoseconds()>;
using DefaultClock = Clock<ClockRatio::nanoseconds()>;

inline DefaultClock& monotonic_clock_mutable()
{
    static DefaultClock clock { DefaultInstant::epoch() };
    return clock;
}

inline const DefaultClock& monotonic_clock()
{
    return monotonic_clock_mutable();
}

}

Chrono::Days operator""d(u64 value) { return Chrono::Days { value }; }
Chrono::Hours operator""h(u64 value) { return Chrono::Hours { value }; }
Chrono::Minutes operator"" m(u64 value) { return Chrono::Minutes { value }; }
Chrono::Seconds operator""s(u64 value) { return Chrono::Seconds { value }; }
Chrono::Milliseconds operator""ms(u64 value) { return Chrono::Milliseconds { value }; }
Chrono::Microseconds operator""us(u64 value) { return Chrono::Microseconds { value }; }
Chrono::Nanoseconds operator""ns(u64 value) { return Chrono::Nanoseconds { value }; }
