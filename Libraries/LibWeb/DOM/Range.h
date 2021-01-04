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

#include <AK/RefCounted.h>
#include <LibWeb/DOM/Node.h>

namespace Web::DOM {

class Position {
public:
    Position(Node& node, size_t offset)
        : m_node(node)
        , m_offset(offset)
    {
    }

    const Node& node() const { return m_node; }
    Node& node() { return m_node; }

    size_t offset() const { return m_offset; }

    bool is_before(const Position& other) const
    {
        return m_node.ptr() == other.m_node.ptr() && m_offset < other.m_offset
            || m_node->is_before(other.m_node);
    }

    bool operator==(const Position& other) const
    {
        return m_node.ptr() == other.m_node.ptr() && m_offset == other.m_offset;
    }

private:
    NonnullRefPtr<Node> m_node;
    size_t m_offset;
};

class Range {
public:
    Range(Position start, Position end)
        : m_start(start)
        , m_end(end)
    {
    }

    explicit Range(Position position)
        : m_start(position)
        , m_end(position)
    {
    }

    const Position& start() const { return m_start; }
    Position& start() { return m_start; }

    const Position& end() const { return m_end; }
    Position& end() { return m_end; }

    bool is_collapsed() const { return m_start == m_end; }

    Range collapse_to_start() const { return Range { m_start }; }
    Range collapse_to_end() const { return Range { m_end }; }

    Range normalized() const
    {
        if (m_end.is_before(m_start))
            return { m_end, m_start };
        return *this;
    }

private:
    Position m_start;
    Position m_end;
};

}
