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

// FIXME: Move this into AK.
#include <LibWeb/TreeNode.h>

namespace Writer {

class RootNode;
class ParagraphNode;
class FragmentNode;

class Node : public Web::TreeNode<Node> {
public:
    virtual ~Node() = default;

    RootNode& root() { return m_root; }
    const RootNode& root() const { return m_root; }

    void inserted_into(Node&) { }
    void children_changed() { }
    void removed_from(Node&) { }

    void dump();

    virtual StringView name() const = 0;
    virtual bool is_child_allowed(Node&) const = 0;
    virtual void dump(StringBuilder&, size_t indent = 0);

    bool dirty() const { return m_dirty; }

protected:
    explicit Node(RootNode& root)
        : m_root(root)
    {
    }

    void flag_dirty() { m_dirty = true; }

private:
    RootNode& m_root;
    bool m_dirty = false;
};

}
