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

#include <AK/HashMap.h>
#include <Applications/Writer/Nodes.h>

namespace Writer {

REGISTER_NODE(ParagraphNode)
REGISTER_NODE(FragmentNode)

static HashMap<String, NodeClassRegistration*>& node_classes()
{
    static HashMap<String, NodeClassRegistration*>* map;
    if (!map)
        map = new HashMap<String, NodeClassRegistration*>;
    return *map;
}

NodeClassRegistration::NodeClassRegistration(const String& class_name, Function<NonnullRefPtr<Node>()> factory)
    : m_class_name(class_name)
    , m_factory(move(factory))
{
    node_classes().set(class_name, this);
}

void NodeClassRegistration::for_each(Function<void(const NodeClassRegistration&)> callback)
{
    for (auto& it : node_classes())
        callback(*it.value);
}

const NodeClassRegistration* NodeClassRegistration::find(const String& class_name)
{
    return node_classes().get(class_name).value_or(nullptr);
}

void Node::load_from_json(StringView json_string)
{
    // FIXME: Error handling.

    auto json_value = JsonValue::from_string(json_string);
    load_from_json(json_value.value().as_object());
}

void Node::load_from_json(const JsonObject& json)
{
    // FIXME: Error handling.

    json.for_each_member([&](auto key, auto value) {
        set_property(key, value);
    });

    const auto& children = json.get("children");
    if (children.is_array()) {
        for (const auto& child_json_value : children.as_array().values()) {
            const auto& child_json = child_json_value.as_object();

            auto* registration = NodeClassRegistration::find(child_json.get("class").as_string());

            if (!registration) {
                dbgln("Missing registration for class {}", child_json.get("class").as_string());
                ASSERT_NOT_REACHED();
            }

            auto child_node = registration->construct();
            add_child(child_node);
            child_node->load_from_json(child_json);
        }
    }
}

}
