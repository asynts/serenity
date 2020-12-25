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
#include <AK/JsonValue.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/Element.h>
#include <LibWeb/Dump.h>
#include <LibWeb/HTML/HTMLElement.h>
#include <LibWeb/Page/Frame.h>

#include <Applications/Writer/Model/DocumentNode.h>
#include <Applications/Writer/Model/FragmentNode.h>
#include <Applications/Writer/Model/Node.h>

namespace Writer {

void Node::dump(StringBuilder& builder, size_t indent)
{
    builder.appendff("{:{}}[{}]\n", "", indent * 2, class_name());

    for_each_child([&](Node& child) {
        child.dump(builder, indent + 1);
    });
}

void Node::dump()
{
    StringBuilder builder;
    dump(builder);
    dbgln("\n{}", builder.string_view());
}

void HeadingNode::render(Badge<Node>)
{
    // FIXME: Make it possible to only re-render this element.

    auto new_element = root().dom().create_element("h3");

    set_element(new_element);
    parent()->element()->append_child(new_element);

    for_each_child([&](Node& node) {
        node.render(node_badge());
    });
}

void HeadingNode::load_from_json(const JsonObject& json)
{
    json.get("children").as_array().for_each([&](const JsonValue& child_json) {
        ASSERT(child_json.as_object().get("class").as_string() == "FragmentNode");

        auto child = create_child<FragmentNode>();
        child->load_from_json(child_json.as_object());
    });
}

JsonValue HeadingNode::export_to_json() const
{
    JsonObject json;

    json.set("class", "HeadingNode");

    JsonArray children;
    for_each_child([&](const Node& child) {
        children.append(child.export_to_json());
    });
    json.set("children", children);

    return json;
}

}
