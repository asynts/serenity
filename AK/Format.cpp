/*
 * Copyright (c) 2020, the SerenityOS developers.
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

#include <AK/Format.h>
#include <AK/PrintfImplementation.h>

namespace AK {

bool Formatter<u32>::parse(String fmt)
{
    if (fmt.length() == 0)
        return true;

    if (fmt[0] != ':')
        return false;

    if (fmt.length() >= 2 && fmt[1] == '0')
        zero_pad = true;

    // Hack alert! The substring_view is null terminated because fmt is null terminated.
    char* endptr = nullptr;
    field_width = strtoul(fmt.substring_view(1).characters_without_null_termination(), &endptr, 10);

    return endptr == fmt.characters() + fmt.length();
}
void Formatter<u32>::format(StringBuilder& builder, u32 value)
{
    char* bufptr;
    PrintfImplementation::print_number([&builder](auto, auto ch) { builder.append(ch); }, bufptr, value, false, zero_pad, field_width);
}

}
