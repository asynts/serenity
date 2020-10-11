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

class SourceGenerator : private GenericLexer {
public:
    using MappingType = HashMap<StringView, String>;

    // SourceGenerator assumes that 'input' is a string literal that appears in the source code, thus it asserts
    // if the input is invalid.
    explicit SourceGenerator(StringView input, const MappingType& mappings, char opening = '@', char closing = '@')
        : GenericLexer(input)
        , m_mappings(mappings)
        , m_opening(opening)
        , m_closing(closing)
    {
    }

    void generate(StringBuilder& builder, const MappingType* override_mappings = nullptr)
    {
        reset();

        while (!is_eof()) {
            // FIXME: It is a bit inconvinient, that 'consume_until' also consumes the 'stop' character, this makes
            //        the method less generic because there is no way to check if the 'stop' character ever appeared.
            const auto consume_until_without_consuming_stop_character = [&](char stop) {
                return consume_while([&](char ch) { return ch != stop; });
            };

            builder.append(consume_until_without_consuming_stop_character(m_opening));

            if (consume_specific(m_opening)) {
                const auto placeholder = consume_until_without_consuming_stop_character(m_closing);

                if (!consume_specific(m_closing))
                    ASSERT_NOT_REACHED();

                Optional<String> optval;

                if (override_mappings)
                    optval = override_mappings->get(placeholder);

                if (!optval.has_value())
                    optval = m_mappings.get(placeholder);

                builder.append(optval.value());
            } else {
                ASSERT(is_eof());
            }
        }
    }

    String generate(const MappingType* override_mappings = nullptr)
    {
        StringBuilder builder;
        generate(builder, override_mappings);
        return builder.build();
    }

private:
    // We are creating a copy to allow something like the following:
    //
    //     SourceGenerator::MappingType mappings;
    //     mappings.set("class", "Foo");
    //
    //     mappings.set("method", "foo");
    //     SourceGenerator method_foo_generator { "const char* @class@::@method@() { return \"foo\"}", mappings };
    //
    //     mappings.set("method", "bar");
    //     SourceGenerator method_foo_generator { "const char* @class@::@method@() { return \"bar\"}", mappings };
    //
    const MappingType m_mappings;

    const char m_opening;
    const char m_closing;
};

}

using AK::SourceGenerator;
