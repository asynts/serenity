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
#include <AK/GenericLexer.h>
#include <AK/String.h>
#include <AK/StringBuilder.h>
#include <ctype.h>

namespace AK {

namespace {

constexpr size_t use_next_index = NumericLimits<size_t>::max();

void vformat_impl(FormatParams& params, FormatBuilder& builder, FormatParser& parser)
{
    const auto literal = parser.consume_literal();
    builder.put_literal(literal);

    FormatParser::FormatSpecifier specifier;
    if (!parser.consume_specifier(specifier)) {
        ASSERT(parser.is_eof());
        return;
    }

    if (specifier.index == use_next_index)
        specifier.index = params.take_next_index();

    auto& parameter = params.parameters().at(specifier.index);

    FormatParser argparser { specifier.flags };
    parameter.formatter(params, builder, argparser, parameter.value);

    vformat_impl(params, builder, parser);
}

} // namespace AK::{anonymous}

size_t FormatParams::decode(size_t value)
{
    if (value == AK::StandardFormatter::value_from_next_arg)
        value = AK::StandardFormatter::value_from_arg + take_next_index();

    if (value >= AK::StandardFormatter::value_from_arg) {
        const auto parameter = parameters().at(value - AK::StandardFormatter::value_from_arg);

        Optional<i64> svalue;
        if (parameter.type == AK::TypeErasedParameter::Type::UInt8)
            value = *reinterpret_cast<const u8*>(parameter.value);
        else if (parameter.type == AK::TypeErasedParameter::Type::UInt16)
            value = *reinterpret_cast<const u16*>(parameter.value);
        else if (parameter.type == AK::TypeErasedParameter::Type::UInt32)
            value = *reinterpret_cast<const u32*>(parameter.value);
        else if (parameter.type == AK::TypeErasedParameter::Type::UInt64)
            value = *reinterpret_cast<const u64*>(parameter.value);
        else if (parameter.type == AK::TypeErasedParameter::Type::Int8)
            svalue = *reinterpret_cast<const i8*>(parameter.value);
        else if (parameter.type == AK::TypeErasedParameter::Type::Int16)
            svalue = *reinterpret_cast<const i16*>(parameter.value);
        else if (parameter.type == AK::TypeErasedParameter::Type::Int32)
            svalue = *reinterpret_cast<const i32*>(parameter.value);
        else if (parameter.type == AK::TypeErasedParameter::Type::Int64)
            svalue = *reinterpret_cast<const i64*>(parameter.value);
        else
            ASSERT_NOT_REACHED();

        if (svalue.has_value()) {
            ASSERT(svalue.value() >= 0);
            value = static_cast<size_t>(svalue.value());
        }
    }

    return value;
}

FormatParser::FormatParser(StringView input)
    : GenericLexer(input)
{
}
StringView FormatParser::consume_literal()
{
    const auto begin = tell();

    while (!is_eof()) {
        if (consume_specific("{{"))
            continue;

        if (consume_specific("}}"))
            continue;

        if (next_is(is_any_of("{}")))
            return m_input.substring_view(begin, tell() - begin);

        consume();
    }

    return m_input.substring_view(begin);
}
bool FormatParser::consume_number(size_t& value)
{
    value = 0;

    bool consumed_at_least_one = false;
    while (next_is(isdigit)) {
        value *= 10;
        value += consume() - '0';
        consumed_at_least_one = true;
    }

    return consumed_at_least_one;
}
bool FormatParser::consume_specifier(FormatSpecifier& specifier)
{
    ASSERT(!next_is('}'));

    if (!consume_specific('{'))
        return false;

    if (!consume_number(specifier.index))
        specifier.index = use_next_index;

    if (consume_specific(':')) {
        const auto begin = tell();

        size_t level = 1;
        while (level > 0) {
            ASSERT(!is_eof());

            if (consume_specific('{')) {
                ++level;
                continue;
            }

            if (consume_specific('}')) {
                --level;
                continue;
            }

            consume();
        }

        specifier.flags = m_input.substring_view(begin, tell() - begin - 1);
    } else {
        if (!consume_specific('}'))
            ASSERT_NOT_REACHED();

        specifier.flags = "";
    }

    return true;
}
bool FormatParser::consume_replacement_field(size_t& index)
{
    if (!consume_specific('{'))
        return false;

    if (!consume_number(index))
        index = use_next_index;

    if (!consume_specific('}'))
        ASSERT_NOT_REACHED();

    return true;
}

void vformat(StringBuilder& builder, StringView fmtstr, Span<const TypeErasedParameter> parameters)
{
    FormatParams params { parameters };
    FormatBuilder fmtbuilder { builder };
    FormatParser parser { fmtstr };

    vformat_impl(params, fmtbuilder, parser);
}
void vformat(const LogStream& stream, StringView fmtstr, Span<const TypeErasedParameter> parameters)
{
    StringBuilder builder;
    vformat(builder, fmtstr, parameters);
    stream << builder.to_string();
}

void StandardFormatter::parse(FormatParams& params, FormatParser& parser)
{
    if (StringView { "<^>" }.contains(parser.peek(1))) {
        ASSERT(!parser.next_is(is_any_of("{}")));
        m_fill = parser.consume();
    }

    if (parser.consume_specific('<'))
        m_align = FormatBuilder::Align::Left;
    else if (parser.consume_specific('^'))
        m_align = FormatBuilder::Align::Center;
    else if (parser.consume_specific('>'))
        m_align = FormatBuilder::Align::Right;

    if (parser.consume_specific('-'))
        m_sign_mode = FormatBuilder::SignMode::OnlyIfNeeded;
    else if (parser.consume_specific('+'))
        m_sign_mode = FormatBuilder::SignMode::Always;
    else if (parser.consume_specific(' '))
        m_sign_mode = FormatBuilder::SignMode::Reserved;

    if (parser.consume_specific('#'))
        m_alternative_form = true;

    if (parser.consume_specific('0'))
        m_zero_pad = true;

    if (size_t index = 0; parser.consume_replacement_field(index)) {
        if (index == use_next_index)
            index = params.take_next_index();

        m_width = value_from_arg + index;
    } else if (size_t width = 0; parser.consume_number(width)) {
        m_width = width;
    }

    if (parser.consume_specific('.')) {
        if (size_t index = 0; parser.consume_replacement_field(index)) {
            if (index == use_next_index)
                index = params.take_next_index();

            m_precision = value_from_arg + index;
        } else if (size_t precision = 0; parser.consume_number(precision)) {
            m_precision = precision;
        }
    }

    if (parser.consume_specific('b'))
        m_mode = Mode::Binary;
    else if (parser.consume_specific('B'))
        m_mode = Mode::BinaryUppercase;
    else if (parser.consume_specific('d'))
        m_mode = Mode::Decimal;
    else if (parser.consume_specific('o'))
        m_mode = Mode::Octal;
    else if (parser.consume_specific('x'))
        m_mode = Mode::Hexadecimal;
    else if (parser.consume_specific('X'))
        m_mode = Mode::HexadecimalUppercase;
    else if (parser.consume_specific('c'))
        m_mode = Mode::Character;
    else if (parser.consume_specific('s'))
        m_mode = Mode::String;
    else if (parser.consume_specific('p'))
        m_mode = Mode::Pointer;

    if (!parser.is_eof())
        dbg() << __PRETTY_FUNCTION__ << " did not consume '" << parser.remaining() << "'";

    ASSERT(parser.is_eof());
}

void Formatter<StringView>::format(FormatParams& params, FormatBuilder& builder, StringView value)
{
    if (m_sign_mode != FormatBuilder::SignMode::Default)
        ASSERT_NOT_REACHED();
    if (m_alternative_form)
        ASSERT_NOT_REACHED();
    if (m_zero_pad)
        ASSERT_NOT_REACHED();
    if (m_mode != Mode::Default && m_mode != Mode::String)
        ASSERT_NOT_REACHED();
    if (m_width != 0 || m_precision != NumericLimits<size_t>::max())
        ASSERT_NOT_REACHED();

    const auto width = params.decode(m_width);
    const auto precision = params.decode(m_precision);

    builder.put_string(value, m_align, width, precision, m_fill);
}

template<typename T>
void Formatter<T, typename EnableIf<IsIntegral<T>::value>::Type>::format(FormatParams& params, FormatBuilder& builder, T value)
{
    if (m_mode == Mode::Character) {
        // FIXME: We just support ASCII for now, in the future maybe unicode?
        ASSERT(value >= 0 && value <= 127);

        m_mode = Mode::String;

        Formatter<StringView> formatter { *this };
        return formatter.format(params, builder, StringView { static_cast<const char*>(&value), 1 });
    }

    if (m_precision != NumericLimits<size_t>::max())
        ASSERT_NOT_REACHED();

    if (m_mode == Mode::Pointer) {
        if (m_sign_mode != Sign::Default)
            ASSERT_NOT_REACHED();
        if (m_align != Align::Default)
            ASSERT_NOT_REACHED();
        if (m_alternative_form)
            ASSERT_NOT_REACHED();
        if (m_width != 0)
            ASSERT_NOT_REACHED();

        m_mode = Mode::Hexadecimal;
        m_alternative_form = true;
        m_width = 2 * sizeof(void*) + 2;
        m_zero_pad = true;
    }

    u8 base = 0;
    bool upper_case = false;
    if (m_mode == Mode::Binary) {
        base = 2;
    } else if (m_mode == Mode::BinaryUppercase) {
        base = 2;
        upper_case = true;
    } else if (m_mode == Mode::Octal) {
        base = 8;
    } else if (m_mode == Mode::Decimal || m_mode == Mode::Default) {
        base = 10;
    } else if (m_mode == Mode::Hexadecimal) {
        base = 16;
    } else if (m_mode == Mode::HexadecimalUppercase) {
        base = 16;
        upper_case = true;
    } else {
        ASSERT_NOT_REACHED();
    }

    const auto width = params.decode(m_width);

    if (IsSame<typename MakeUnsigned<T>::Type, T>::value)
        builder.put_u64(value, base, m_alternative_form, upper_case, m_zero_pad, m_align, width, m_fill, m_sign_mode);
    else
        builder.put_i64(value, base, m_alternative_form, upper_case, m_zero_pad, m_align, width, m_fill, m_sign_mode);
}

void Formatter<bool>::format(FormatParams& params, FormatBuilder& builder, bool value)
{
    if (m_mode == Mode::Binary || m_mode == Mode::BinaryUppercase || m_mode == Mode::Decimal || m_mode == Mode::Octal || m_mode == Mode::Hexadecimal || m_mode == Mode::HexadecimalUppercase) {
        Formatter<u8> formatter { *this };
        return formatter.format(params, builder, static_cast<u8>(value));
    } else {
        Formatter<StringView> formatter { *this };
        return formatter.format(params, builder, value ? "true" : "false");
    }
}

template struct Formatter<unsigned char, void>;
template struct Formatter<unsigned short, void>;
template struct Formatter<unsigned int, void>;
template struct Formatter<unsigned long, void>;
template struct Formatter<unsigned long long, void>;
template struct Formatter<char, void>;
template struct Formatter<short, void>;
template struct Formatter<int, void>;
template struct Formatter<long, void>;
template struct Formatter<long long, void>;
template struct Formatter<signed char, void>;

} // namespace AK
