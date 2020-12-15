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

#include <AK/HashMap.h>
#include <AK/String.h>
#include <LibWeb/DOM/Element.h>

// FIXME: Move this into AK.
#include <LibWeb/TreeNode.h>

namespace Writer {

class DocumentNode;

class Node : public Web::TreeNode<Node> {
public:
    virtual ~Node() = default;

    void will_be_destroyed()
    {
        remove_all_children();
    }

    DocumentNode& root() { return m_root; }
    const DocumentNode& root() const { return m_root; }

    const Web::DOM::Element* element() const { return m_element; }
    Web::DOM::Element* element() { return m_element; }

    void set_element(Web::DOM::Element& value) { m_element = &value; }

    template<typename T, typename... Parameters>
    NonnullRefPtr<T> create_child(Parameters&&... parameters)
    {
        auto node = T::create(root(), forward<Parameters>(parameters)...);
        append_child(node);
        return node;
    }

    // FIXME: Can we leverage this?
    void inserted_into(Node&)
    {
        // FIXME: Render!
    }

    void children_changed() { }

    void removed_from(Node&);

    virtual void render() = 0;
    virtual void load_from_json(const JsonObject&) = 0;
    virtual JsonValue export_to_json() const = 0;
    virtual StringView class_name() const = 0;

protected:
    void replace_element_with(Web::DOM::Element& new_element);

    explicit Node(DocumentNode& root)
        : m_root(root)
    {
    }

private:
    RefPtr<Web::DOM::Element> m_element;
    DocumentNode& m_root;
};

class DocumentNode final : public Node {
public:
    static NonnullRefPtr<DocumentNode> create(Web::DOM::Document& document)
    {
        return adopt(*new DocumentNode { document });
    }

    static NonnullRefPtr<DocumentNode> create_from_json(Web::DOM::Document&, StringView json);
    static NonnullRefPtr<DocumentNode> create_from_json(Web::DOM::Document&, const JsonObject&);
    static NonnullRefPtr<DocumentNode> create_from_file(Web::DOM::Document&, StringView path);

    const Web::DOM::Document& dom() const { return m_dom; }
    Web::DOM::Document& dom() { return m_dom; }

    void render() override;
    void load_from_json(const JsonObject&) override;
    JsonValue export_to_json() const override;
    StringView class_name() const override { return "DocumentNode"; }

    void write_to_file(StringView path);

    Node* lookup(Web::DOM::Node& node) const
    {
        return m_lookup.get(&node).value_or(nullptr);
    }

    void add_lookup(Web::DOM::Node& web_node, Node& writer_node)
    {
        m_lookup.set(&web_node, &writer_node);
    }

private:
    explicit DocumentNode(Web::DOM::Document& dom)
        : Node(*this)
        , m_dom(dom)
    {
    }

    HashMap<Web::DOM::Node*, Node*> m_lookup;
    NonnullRefPtr<Web::DOM::Document> m_dom;
};

class ParagraphNode final : public Node {
public:
    static NonnullRefPtr<ParagraphNode> create(DocumentNode& document)
    {
        return adopt(*new ParagraphNode { document });
    }

    void merge(ParagraphNode&);

    void render() override;
    void load_from_json(const JsonObject&) override;
    JsonValue export_to_json() const override;
    StringView class_name() const override { return "ParagraphNode"; }

private:
    using Node::Node;
};

class FragmentNode final : public Node {
public:
    static NonnullRefPtr<FragmentNode> create(DocumentNode& document)
    {
        return adopt(*new FragmentNode { document });
    }

    void render() override;
    void load_from_json(const JsonObject&) override;
    JsonValue export_to_json() const override;
    StringView class_name() const override { return "FragmentNode"; }
    void remove_content(size_t offset, size_t length);
    void remove_content(size_t offset);

    String content() const { return m_content; }
    void set_content(String value)
    {
        m_content = value;
        // FIXME: We want to call render() here.
    }

    bool bold() const { return m_bold; }
    void set_bold(bool value)
    {
        m_bold = value;
        // FIXME: We want to call render() here.
    }

private:
    using Node::Node;

    String m_content = "";
    bool m_bold = false;
};

}

AK_BEGIN_TYPE_TRAITS(Writer::DocumentNode)
static bool is_type(const Writer::Node& node) { return node.class_name() == "DocumentNode"; }
AK_END_TYPE_TRAITS()

AK_BEGIN_TYPE_TRAITS(Writer::ParagraphNode)
static bool is_type(const Writer::Node& node) { return node.class_name() == "ParagraphNode"; }
AK_END_TYPE_TRAITS()

AK_BEGIN_TYPE_TRAITS(Writer::FragmentNode)
static bool is_type(const Writer::Node& node) { return node.class_name() == "FragmentNode"; }
AK_END_TYPE_TRAITS()
