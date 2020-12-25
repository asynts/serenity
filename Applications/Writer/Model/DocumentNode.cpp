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

#include <Applications/Writer/Model/DocumentNode.h>
#include <Applications/Writer/Model/HeadingNode.h>
#include <Applications/Writer/Model/ParagraphNode.h>

namespace Writer {

void DocumentNode::render(Badge<Node>)
{
    ASSERT_NOT_REACHED();
}

void DocumentNode::render()
{
    element()->remove_all_children();

    for_each_child([&](Node& node) {
        node.render(node_badge());
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

}
