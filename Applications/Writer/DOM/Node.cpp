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

#include <AK/JsonObject.h>
#include <Applications/Writer/DOM/Node.h>
#include <Applications/Writer/Loader.h>
#include <LibWeb/DOM/Element.h>

namespace Writer::DOM {

void Node::destroy_rendered()
{
    if (m_rendered) {
        if (m_rendered->parent())
            m_rendered->parent()->remove_child(*m_rendered);

        m_rendered = nullptr;
    }
}

RefPtr<DOM::Node> Node::load_from_json(StringView input)
{
    auto json_value = JsonValue::from_string(input);

    ASSERT(json_value.has_value());
    ASSERT(json_value.value().is_object());

    return load_from_json(json_value.value().as_object());
}

RefPtr<DOM::Node> Node::load_from_json(const JsonObject& json)
{
    auto class_name = json.get_ptr("class");

    ASSERT(class_name);
    ASSERT(class_name->is_string());

    auto object = NodeRegistration::registrations().get(class_name->as_string()).value()->constructor();

    json.for_each_member([](String property, JsonValue value) {
        // FIXME: How?
    });
}

void Node::register_in_loader()
{
}

void ParagraphNode::register_in_loader()
{
    static bool registered = false;

    if (!exchange(registered, true)) {
        auto registration = make<NodeRegistration>();
        registration->name = "ParagraphNode";

        NodeRegistration::registrations().set(registration->name, move(registration));
    }
}

void FragmentNode::register_in_loader()
{
    static bool registered = false;

    if (!exchange(registered, true)) {
        auto registration = make<NodeRegistration>();
        registration->name = "FragmentNode";

        registration->properties.set("content", { "content", content, set_content });
        registration->properties.set("bold", { "bold", bold, set_bold });
        registration->properties.set("italic", { "italic", italic, set_italic });

        NodeRegistration::registrations().set(registration->name, move(registration));
    }
}

}
