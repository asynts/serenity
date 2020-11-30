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

#include <Applications/Writer/DOM/Node.h>

namespace Writer::DOM {

static NonnullRefPtr<Node> decode_document(const JsonObject& json)
{
    if (json.get("type").as_string() == "doc") {
        auto element = adopt(*new DocumentNode);

        // FIXME: This child.to_string() or child.as_object() crashes here?
        for (const auto& child : json.get("tree").as_array().values())
            element->append_child(decode_document(child.as_object()));

        return element;
    } else if (json.get("type").as_string() == "para") {
        auto element = adopt(*new ParagraphNode);

        for (const auto& child : json.get("tree").as_array().values())
            element->append_child(decode_document(child.as_object()));

        return element;
    } else if (json.get("type").as_string() == "frag") {
        auto element = adopt(*new FragmentNode);

        element->set_content(json.get("content").as_string());

        if (json.get("b").as_bool())
            element->set_bold(true);
        if (json.get("i").as_bool())
            element->set_italic(true);

        for (const auto& child : json.get("tree").as_array().values())
            element->append_child(decode_document(child.as_object()));

        return element;
    } else {
        ASSERT_NOT_REACHED();
    }
}

NonnullRefPtr<DocumentNode> DocumentNode::from_json(StringView input)
{
    return from_json(JsonValue::from_string(input).value().as_object());
}

NonnullRefPtr<DocumentNode> DocumentNode::from_json(const JsonObject& json)
{
    return downcast<DocumentNode>(*decode_document(json));
}

JsonValue DocumentNode::to_json()
{
    JsonObject object;
    object.set("type", "doc");

    JsonArray tree;
    object.set("tree", tree);

    for_each_child([&](Node& node) { tree.append(node.to_json()); });

    return object;
}

NonnullRefPtr<Layout::Block> DocumentNode::layout() const
{
    auto block = adopt(*new Layout::VerticalBlock);

    for_each_child([&](const Node& node) {
        block->append_child(node.layout());

        return IterationDecision::Continue;
    });

    return block;
}

JsonValue ParagraphNode::to_json()
{
    JsonObject object;
    object.set("type", "para");

    JsonArray tree;
    object.set("tree", tree);

    for_each_child([&](Node& node) { tree.append(node.to_json()); });

    return object;
}

NonnullRefPtr<Layout::Block> ParagraphNode::layout() const
{
    // FIXME: Line wraping?

    auto block = adopt(*new Layout::HorizontalBlock);

    for_each_child([&](const Node& node) {
        block->append_child(node.layout());

        return IterationDecision::Continue;
    });

    return block;
}

JsonValue FragmentNode::to_json()
{
    JsonObject object;
    object.set("type", "frag");
    object.set("c", content());
    object.set("b", bold());
    object.set("i", italic());

    return object;
}

NonnullRefPtr<Layout::Block> FragmentNode::layout() const
{
    // FIXME: How to render italic text?

    if (bold())
        return adopt(*new Layout::TextBlock { Gfx::Font::default_bold_font(), content() });
    else
        return adopt(*new Layout::TextBlock { Gfx::Font::default_font(), content() });
}

}
