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

#include <AK/Assertions.h>
#include <AK/Chrono.h>
#include <AK/StringBuilder.h>
#include <time.h>

namespace Chrono {

NanosecondsSinceEpoch MonotonicClock::now()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
        ASSERT_NOT_REACHED();

    Seconds seconds { ts.tv_sec };
    Nanoseconds nanoseconds { ts.tv_nsec };

    return NanosecondsSinceEpoch::epoch() + seconds.in_nanoseconds() + nanoseconds;
}

NanosecondsSinceEpoch RealtimeClock::now()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0)
        ASSERT_NOT_REACHED();

    Seconds seconds { ts.tv_sec };
    Nanoseconds nanoseconds { ts.tv_nsec };

    return NanosecondsSinceEpoch::epoch() + seconds.in_nanoseconds() + nanoseconds;
}

}

void AK::Formatter<Chrono::Nanoseconds>::format(FormatBuilder& fmtbuilder, Chrono::Nanoseconds value)
{
    GenericLexer lexer { m_specifier };

    Chrono::SplittedDuration splitted { value };

    StringBuilder& builder = fmtbuilder.builder();
    while (!lexer.is_eof()) {
        if (lexer.consume_specific('%')) {
            bool zero_pad = lexer.consume_specific('0');

            switch (lexer.consume()) {
            case '%':
                builder.append('%');
                break;
            case 'H':
                builder.appendff("{:02}", splitted.hours().ticks());
                break;
            case 'M':
                builder.appendff("{:02}", splitted.minutes().ticks());
                break;
            case 'S':
                builder.appendff("{:02}", splitted.seconds().ticks());
                break;
            case 'm':
                ASSERT(!zero_pad);
                builder.appendff("{:03}", splitted.milliseconds().ticks());
                break;
            case 'u':
                ASSERT(!zero_pad);
                builder.appendff("{:06}", (splitted.microseconds() + splitted.milliseconds().in_microseconds()).ticks());
                break;
            case 'n':
                ASSERT(!zero_pad);
                builder.appendff("{:09}", (splitted.nanoseconds() + splitted.microseconds().in_nanoseconds() + splitted.milliseconds().in_nanoseconds()).ticks());
                break;
            default:
                ASSERT_NOT_REACHED();
            }
        } else {
            builder.append(lexer.consume());
        }
    }
}
