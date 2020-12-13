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
#include <LibCore/File.h>
#include <LibCore/FileStream.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/Element.h>
#include <LibWeb/HTML/HTMLElement.h>

#include <Applications/Writer/Model/Node.h>

namespace Writer {

void Node::removed_from(Node&)
{
    if (m_element) {
        m_element->parent()->remove_child(*m_element);
        m_element.clear();
    }
}

void Node::replace_element_with(Web::DOM::Element& new_element)
{
    if (element())
        element()->replace_with(new_element);
    else
        parent()->element()->append_child(new_element);

    root().add_lookup(new_element, *this);
    m_element = new_element;
}

void DocumentNode::render()
{
    for_each_child([&](Node& node) {
        node.render();
    });
}

NonnullRefPtr<DocumentNode> DocumentNode::create_from_json(Web::DOM::Document& document, StringView json)
{
    return create_from_json(document, JsonValue::from_string(json).value().as_object());
}

NonnullRefPtr<DocumentNode> DocumentNode::create_from_json(Web::DOM::Document& document, const JsonObject& json)
{
    ASSERT(json.get("class").as_string() == "DocumentNode");

    auto node = DocumentNode::create(document);
    node->load_from_json(json);
    return node;
}

void DocumentNode::load_from_json(const JsonObject& json)
{
    json.get("children").as_array().for_each([&](const JsonValue& child_json) {
        ASSERT(child_json.as_object().get("class").as_string() == "ParagraphNode");

        auto child = create_child<ParagraphNode>();
        child->load_from_json(child_json.as_object());
    });
}

JsonValue DocumentNode::export_to_json() const
{
    JsonObject json;

    json.set("class", "DocumentNode");

    JsonArray children;
    for_each_child([&](const Node& child) {
        children.append(child.export_to_json());
    });
    json.set("children", children);

    return json;
}

void DocumentNode::write_to_file(StringView path)
{
    // FIXME: Error handling.
    auto stream = Core::OutputFileStream::open(path).value();
    stream.write_or_error(export_to_json().to_string().bytes());
}

NonnullRefPtr<DocumentNode> DocumentNode::create_from_file(Web::DOM::Document& document, StringView path)
{
    // FIXME: Error handling.
    auto file = Core::File::open(path, Core::IODevice::OpenMode::ReadOnly).value();
    auto json_string = String { file->read_all().bytes(), AK::ShouldChomp::NoChomp };

    return DocumentNode::create_from_json(document, json_string);
}

void ParagraphNode::render()
{
    auto new_element = root().dom().create_element("p");

    replace_element_with(new_element);

    for_each_child([&](Node& node) {
        node.render();
    });
}

void ParagraphNode::load_from_json(const JsonObject& json)
{
    json.get("children").as_array().for_each([&](const JsonValue& child_json) {
        ASSERT(child_json.as_object().get("class").as_string() == "FragmentNode");

        auto child = create_child<FragmentNode>();
        child->load_from_json(child_json.as_object());
    });
}

JsonValue ParagraphNode::export_to_json() const
{
    JsonObject json;

    json.set("class", "ParagraphNode");

    JsonArray children;
    for_each_child([&](const Node& child) {
        children.append(child.export_to_json());
    });
    json.set("children", children);

    return json;
}

void FragmentNode::render()
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

    replace_element_with(new_element);
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

}
