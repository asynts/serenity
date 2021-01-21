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

    static constexpr ClockRatio optimal() { TODO(); }
    static constexpr ClockRatio seconds() { return { 1, 1 }; }
    static constexpr ClockRatio milliseconds() { return { 1, 1'000 }; }
    static constexpr ClockRatio microseconds() { return { 1, 1'000'000 }; }
    static constexpr ClockRatio nanoseconds() { return { 1, 1'000'000'000 }; }

    constexpr bool operator==(ClockRatio rhs) const { TODO(); }
};

template<ClockRatio Ratio = ClockRatio::optimal()>
class Duration {
public:
    explicit Duration(i64 ticks)
        : m_ticks(ticks)
    {
    }

    static Duration zero() { return { 0 }; }

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

template<ClockRatio Ratio>
class Instant {
public:
    Instant(u64 ticks)
        : m_ticks(ticks)
    {
    }

    static Instant epoch() { return { 0 }; }

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

// FIXME: Implement.

class Clock {
public:
    Clock(ClockRatio, Instant);

    Instant now() const;
    ClockRatio ratio() const;

    void tick(u64 delta = 1);
};

class Stopwatch {
public:
    Stopwatch();

    Duration elapsed() const;

    void start();
    void pause();
    void reset();
};

class Timer {
public:
    Timer();
    explicit Timer(Duration, bool single_shot = true);

    Duration duration() const;

    void start();
    void start(Duration, bool single_shot = true);

    void stop();
    void stop(Duration);

    Function<void(Instant)> on_timeout;
};

}
