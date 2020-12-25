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

#include <Applications/Writer/Model/Node.h>

namespace Writer {

class FragmentNode final : public Node {
public:
    static NonnullRefPtr<FragmentNode> create(DocumentNode& document)
    {
        return adopt(*new FragmentNode { document });
    }

    void render(Badge<Node>) override;
    void load_from_json(const JsonObject&) override;
    JsonValue export_to_json() const override;
    StringView class_name() const override { return "FragmentNode"; }
    void dump(StringBuilder& builder, size_t indent = 0) override;

    void remove_content(size_t offset, size_t length);
    void remove_content(size_t offset);

    void insert_content(size_t offset, StringView);

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

AK_BEGIN_TYPE_TRAITS(Writer::FragmentNode)
static bool is_type(const Writer::Node& node) { return node.class_name() == "FragmentNode"; }
AK_END_TYPE_TRAITS()
