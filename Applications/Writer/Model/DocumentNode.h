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

#include <LibWeb/DOM/Document.h>

#include <Applications/Writer/Model/Node.h>
#include <Applications/Writer/Model/Range.h>

namespace Writer {

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

    void render(Badge<Node>) override;
    void render();

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

    Range* selection()
    {
        if (m_selection.has_value())
            return &m_selection.value();

        return nullptr;
    }

    void set_selection(Range value)
    {
        m_selection = value;
    }

    Position* cursor()
    {
        if (m_selection.has_value())
            return &m_selection.value().end();

        return nullptr;
    }

    void set_cursor(Position value)
    {
        m_selection = Range { value };
    }

private:
    explicit DocumentNode(Web::DOM::Document& dom)
        : Node(*this)
        , m_dom(dom)
    {
    }

    HashMap<Web::DOM::Node*, Node*> m_lookup;
    NonnullRefPtr<Web::DOM::Document> m_dom;
    Optional<Range> m_selection;
};

}
