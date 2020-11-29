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

#include <AK/JsonObject.h>
#include <AK/String.h>
#include <Applications/Writer/Layout/Block.h>
#include <LibGfx/Painter.h>
#include <LibWeb/TreeNode.h>

namespace Writer::DOM {

enum class NodeType {
    Document,
    Paragraph,
    Fragment,
};

class Node : public Web::TreeNode<Node> {
public:
    virtual ~Node() = default;

    NodeType type() const { return m_type; }

    virtual JsonValue to_json() = 0;

    void inserted_into(const Node&) { }
    void children_changed() { }

    virtual NonnullRefPtr<Layout::Block> layout() const = 0;

protected:
    explicit Node(NodeType type)
        : m_type(type)
    {
    }

private:
    NodeType m_type;
};

class DocumentNode final : public Node {
public:
    DocumentNode()
        : Node(NodeType::Document)
    {
    }

    JsonValue to_json() override;
    Gfx::IntRect draw(Gfx::Painter&, Gfx::IntPoint);
    NonnullRefPtr<Layout::Block> layout() const override;

    static NonnullRefPtr<DocumentNode> from_json(const JsonObject&);
};

class ParagraphNode final : public Node {
public:
    ParagraphNode()
        : Node(NodeType::Paragraph)
    {
    }

    JsonValue to_json() override;
    Gfx::IntRect draw(Gfx::Painter&, Gfx::IntPoint);
    NonnullRefPtr<Layout::Block> layout() const override;
};

class FragmentNode final : public Node {
public:
    FragmentNode()
        : Node(NodeType::Fragment)
    {
    }

    JsonValue to_json() override;
    Gfx::IntRect draw(Gfx::Painter&, Gfx::IntPoint);
    NonnullRefPtr<Layout::Block> layout() const override;

    String content() const { return m_content; }
    void set_content(String value) { m_content = value; }

    bool bold() const { return m_bold; }
    void set_bold(bool value) { m_bold = value; }

    bool italic() const { return m_italic; }
    void set_italic(bool value) { m_italic = value; }

private:
    String m_content = "";
    bool m_bold = false;
    bool m_italic = false;
};

}

AK_BEGIN_TYPE_TRAITS(Writer::DOM::DocumentNode)
static bool is_type(const Writer::DOM::Node& node) { return node.type() == Writer::DOM::NodeType::Document; }
AK_END_TYPE_TRAITS()

AK_BEGIN_TYPE_TRAITS(Writer::DOM::ParagraphNode)
static bool is_type(const Writer::DOM::Node& node) { return node.type() == Writer::DOM::NodeType::Paragraph; }
AK_END_TYPE_TRAITS()

AK_BEGIN_TYPE_TRAITS(Writer::DOM::FragmentNode)
static bool is_type(const Writer::DOM::Node& node) { return node.type() == Writer::DOM::NodeType::Fragment; }
AK_END_TYPE_TRAITS()
