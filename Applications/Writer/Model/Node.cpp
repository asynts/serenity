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

void FragmentNode::render(Badge<Node>)
{
    auto new_element = root().dom().create_element("span");

    new_element->set_text_content(m_content);

    if (m_bold)
        new_element->class_names().append("bold");

    // There are text nodes created automatically, these also belong to this node.
    new_element->for_each_in_subtree([&](Web::DOM::Node& node) {
        root().add_lookup(node, *this);
        return IterationDecision::Continue;
    });

    set_element(new_element);
    parent()->element()->append_child(new_element);
}

void FragmentNode::load_from_json(const JsonObject& json)
{
    set_content(json.get("content").as_string());
    set_bold(json.get("bold").as_bool());

    ASSERT(json.get("children").is_null());
}

JsonValue FragmentNode::export_to_json() const
{
    JsonObject json;

    json.set("class", "FragmentNode");
    json.set("content", m_content);
    json.set("bold", m_bold);

    return json;
}

void FragmentNode::remove_content(size_t offset, size_t length)
{
    if (length == 0)
        return;

    StringBuilder builder;
    builder.append(m_content.substring(0, offset));
    builder.append(m_content.substring(offset + length, m_content.length() - offset - length));

    set_content(builder.build());
}

void FragmentNode::remove_content(size_t offset)
{
    remove_content(offset, m_content.length() - offset);
}

void FragmentNode::insert_content(size_t offset, StringView snippet)
{
    StringBuilder builder;
    builder.append(m_content.substring(0, offset));
    builder.append(snippet);
    builder.append(m_content.substring(offset));

    set_content(builder.build());
}

void FragmentNode::dump(StringBuilder& builder, size_t indent)
{
    builder.appendff("{:{}}[{}] bold={}\n", "", indent * 2, class_name(), bold());
    builder.appendff("{:{}}{}\n", "", (indent + 1) * 2, content());
}

}
