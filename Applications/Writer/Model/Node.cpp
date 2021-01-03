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
#include <AK/StringBuilder.h>

#include <Applications/Writer/Model/FragmentNode.h>
#include <Applications/Writer/Model/Node.h>
#include <Applications/Writer/Model/ParagraphNode.h>

namespace Writer {

void Node::dump(StringBuilder& builder, size_t indent)
{
    builder.appendff("{:{}}[{}]\n", "", indent * 2, name());

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

bool Node::load_from_json(const JsonObject& object)
{
    if (object.get("name").as_string_or("") != name())
        return false;

    if (!object.get("children").is_null()) {
        if (!object.get("children").is_array())
            return false;

        auto children = object.get("children").as_array();
        for (auto& child_json : children.values()) {
            if (!child_json.is_object())
                return false;
            auto& child_object = child_json.as_object();

            auto child = construct_node_dynamically(root(), child_object.get("name").as_string_or(""));
            if (!child)
                return false;

            if (!child->load_from_json(child_object))
                return false;

            if (!append_child(*child))
                return false;
        }
    }

    return true;
}

RefPtr<Node> construct_node_dynamically(RootNode& root, StringView name)
{
    if (name == "ParagraphNode")
        return ParagraphNode::create(root);
    if (name == "FragmentNode")
        return FragmentNode::create(root);

    return nullptr;
}

}
