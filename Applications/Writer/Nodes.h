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

// FIXME: Why do I need this include here?
#include <AK/JsonObject.h>

#include <AK/NeverDestroyed.h>
#include <LibCore/Object.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/Element.h>

#define REGISTER_NODE(class_name) \
    NeverDestroyed<NodeClassRegistration> registration_##class_name { #class_name, [](auto& document, auto& parent) { return class_name::construct(document, parent); } };

namespace Writer {

class Node;

class NodeClassRegistration {
    AK_MAKE_NONCOPYABLE(NodeClassRegistration);
    AK_MAKE_NONMOVABLE(NodeClassRegistration);

public:
    NodeClassRegistration(const String& class_name, Function<NonnullRefPtr<Node>(Web::DOM::Document&, Node&)> factory);

    static void for_each(Function<void(const NodeClassRegistration&)>);
    static const NodeClassRegistration* find(const String& class_name);

    String class_name() const { return m_class_name; }
    NonnullRefPtr<Node> construct(Web::DOM::Document& document, Node& parent) const
    {
        return m_factory(document, parent);
    }

private:
    String m_class_name;
    Function<NonnullRefPtr<Node>(Web::DOM::Document&, Node&)> m_factory;
};

class Node : public Core::Object {
    C_OBJECT(Node);

public:
    explicit Node(Web::DOM::Document& document, Node* parent = nullptr, Web::DOM::Element* element = nullptr)
        : m_element(element)
        , m_parent(parent)
        , m_document(document)
    {
    }
    ~Node()
    {
        if (m_parent)
            m_parent->element()->remove_child(*m_element);
    }

    void load_from_json(StringView);
    void load_from_json(const JsonObject&);

    const RefPtr<Web::DOM::Element>& element() const { return m_element; }
    RefPtr<Web::DOM::Element>& element() { return m_element; }

protected:
    RefPtr<Web::DOM::Element> m_element;
    Node* m_parent;
    Web::DOM::Document& m_document;
};

class ParagraphNode : public Node {
    C_OBJECT(ParagraphNode);

public:
    explicit ParagraphNode(Web::DOM::Document& document, Node& parent)
        : Node(document, &parent)
    {
        // FIXME: I need to put this into a render() method and then figure out how to
        //        invalidate all children...

        m_element = document.create_element("p");
        parent.element()->append_child(*m_element);
    }
};

class FragmentNode : public Node {
    C_OBJECT(FragmentNode);

public:
    FragmentNode(Web::DOM::Document& document, Node& parent)
        : Node(document, &parent)
    {
        REGISTER_BOOL_PROPERTY("bold", bold, set_bold);
        REGISTER_STRING_PROPERTY("content", content, set_content);

        render();
    }

    bool bold() const { return m_bold; }
    void set_bold(bool value)
    {
        m_bold = value;
        render();
    }

    String content() const { return m_content; }
    void set_content(const String& value)
    {
        m_content = value;
        render();
    }

private:
    void render()
    {
        auto element = m_document.create_element("span");
        element->set_text_content(m_content);

        if (m_bold)
            element->class_names().append("bold");

        if (m_element) {
            // FIXME: Somehow this leaves the DOM in an invalid state.
            m_element->replace_with(element);
        } else {
            m_parent->element()->append_child(*element);
        }

        m_element = element;
    }

    bool m_bold = false;
    String m_content = "";
};

}
