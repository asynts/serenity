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
#include <LibWeb/Dump.h>
#include <LibWeb/HTML/HTMLElement.h>
#include <LibWeb/Page/Frame.h>

#include <Applications/Writer/Model/Node.h>

namespace Writer {

void Node::removed_from(Node&)
{
    if (m_element) {
        if (m_element->parent())
            m_element->parent()->remove_child(*m_element);

        m_element->remove_all_children();
        m_element.clear();
    }

    // FIXME: Finally deal with the stale layout node issue.
    root().dom().force_layout();
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
        const auto class_ = child_json.as_object().get("class").as_string();

        if (class_ == "ParagraphNode") {
            auto child = create_child<ParagraphNode>();
            child->load_from_json(child_json.as_object());
        } else if (class_ == "HeadingNode") {
            auto child = create_child<HeadingNode>();
            child->load_from_json(child_json.as_object());
        } else {
            ASSERT_NOT_REACHED();
        }
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

void ParagraphNode::merge(ParagraphNode& other)
{
    if (this == &other)
        return;

    other.for_each_child([&](Node& child) {
        adopt_child(child);

        // FIXME: This should happen in inserted_into.
        child.render();
    });

    other.parent()->remove_child(other);
}

void HeadingNode::render()
{
    auto new_element = root().dom().create_element("h3");

    replace_element_with(new_element);

    for_each_child([&](Node& node) {
        node.render();
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

    // FIXME: Hack. When we replace a node we have to ensure that the cursor isn't
    //        "lost" in the old object.
    auto& cursor_position = root().dom().frame()->cursor_position();
    if (cursor_position.node() && cursor_position.node()->parent() == element()) {
        ASSERT(element() && element()->child_count() == 1);
        ASSERT(new_element->child_count() == 1);
        root().dom().frame()->set_cursor_position(Web::DOM::Position { *new_element->first_child(), cursor_position.offset() });
    }

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

void FragmentNode::remove_content(size_t offset, size_t length)
{
    if (length == 0)
        return;

    StringBuilder builder;
    builder.append(m_content.substring(0, offset));
    builder.append(m_content.substring(offset + length, m_content.length() - offset - length));

    set_content(builder.build());

    // FIXME: This should happen in set_content.
    render();
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

    // FIXME: This should happen in set_content.
    render();
}

}
