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

#include <AK/HashMap.h>
#include <AK/OwnPtr.h>
#include <AK/String.h>
#include <Applications/Writer/Forward.h>
#include <LibCore/Property.h>

namespace Writer {

struct Property {
    String name;
    Function<JsonValue(DOM::Node&)> getter;
    Function<void(DOM::Node&, const JsonValue&)> setter;
};

struct NodeRegistration {
    static HashMap<String, const NonnullOwnPtr<NodeRegistration>>& registrations();

    String name;
    Function<NonnullRefPtr<DOM::Node&>()> create;
    HashMap<String, Property> properties;
};

template<typename T, typename S>
Function<JsonValue(const DOM::Node&)> getter(S (T::*method)() const)
{
    return [method](const DOM::Node& node) {
        return (static_cast<const T&>(node).*(method))();
    };
}

template<typename T, typename S>
Function<void(DOM::Node&, const JsonValue&)> setter(void (T::*method)(S))
{
    return [method](DOM::Node& node, const JsonValue& json) {
        if constexpr (IsSame<S, String>::value)
            (static_cast<T&>(node).*(method))(json.as_string());
        else if constexpr (IsSame<S, bool>::value)
            (static_cast<T&>(node).*(method))(json.as_bool());
        else
            ASSERT_NOT_REACHED();
    };
}

template<typename T>
Function<RefPtr<DOM::Node>()> constructor()
{
    return [](DOM::Document& document) {
        return adopt(new T { document });
    };
}

}
