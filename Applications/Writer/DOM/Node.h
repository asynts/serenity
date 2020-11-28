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

#include <AK/String.h>
#include <Applications/Writer/Forward.h>
#include <LibWeb/Forward.h>
#include <LibWeb/TreeNode.h>

namespace Writer::DOM {

// FIXME: Web::TreeNode should be moved into AK and Core::Object should inherit from it.
class Node : public Web::TreeNode<Node> {
public:
    virtual void render() = 0;
    virtual void destroy_rendered();

    Node(Document& document)
        : m_document(document)
    {
    }

    static RefPtr<DOM::Node> load_from_json(StringView);
    static RefPtr<DOM::Node> load_from_json(const JsonObject&);

    static void register_in_loader();

protected:
    Document& m_document;
    Web::DOM::Element* m_rendered = nullptr;
};

class ParagraphNode : public Node {
public:
    using Node::Node;

    static void register_in_loader();
};

class FragmentNode : public Node {
public:
    using Node::Node;

    static void register_in_loader();

    void set_content(String value) { m_content = value; }
    String content() const { return m_content; }

    void set_bold(bool value) { m_bold = value; }
    bool bold() const { return m_bold; }

    void set_italic(bool value) { m_italic = value; }
    bool italic() const { return m_italic; }

private:
    String m_content = "";
    bool m_bold = false;
    bool m_italic = false;
};

}
