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

#pragma once

#include <AK/GenericLexer.h>
#include <AK/HashMap.h>
#include <AK/String.h>
#include <AK/StringBuilder.h>

namespace AK {

class SourceGenerator : public StringBuilder {
    AK_MAKE_NONCOPYABLE(SourceGenerator);

public:
    using MappingType = HashMap<StringView, String>;

    explicit SourceGenerator(const MappingType& mapping, char opening = '@', char closing = '@')
        : m_mapping(mapping)
        , m_opening(opening)
        , m_closing(closing)
    {
    }
    explicit SourceGenerator(char opening = '@', char closing = '@')
        : m_opening(opening)
        , m_closing(closing)
    {
    }

    void set(StringView key, String value) { m_mapping.set(key, value); }

    void append_pattern(StringView pattern, const MappingType* override_mapping = nullptr)
    {
        GenericLexer lexer { pattern };

        while (!lexer.is_eof()) {
            // FIXME: It is a bit inconvinient, that 'consume_until' also consumes the 'stop' character, this makes
            //        the method less generic because there is no way to check if the 'stop' character ever appeared.
            const auto consume_until_without_consuming_stop_character = [&](char stop) {
                return lexer.consume_while([&](char ch) { return ch != stop; });
            };

            append(consume_until_without_consuming_stop_character(m_opening));

            if (lexer.consume_specific(m_opening)) {
                const auto placeholder = consume_until_without_consuming_stop_character(m_closing);

                if (!lexer.consume_specific(m_closing))
                    ASSERT_NOT_REACHED();

                Optional<String> optval;

                if (override_mapping)
                    optval = override_mapping->get(placeholder);

                if (!optval.has_value())
                    optval = m_mapping.get(placeholder);

                append(optval.value());
            } else {
                ASSERT(lexer.is_eof());
            }
        }
    }

    String generate() const { return build(); }

    const MappingType& mapping() const { return m_mapping; }
    MappingType& mapping() { return m_mapping; }

private:
    MappingType m_mapping;
    char m_opening, m_closing;
};

}

using AK::SourceGenerator;
