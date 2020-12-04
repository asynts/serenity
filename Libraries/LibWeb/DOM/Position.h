/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
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

#include <AK/RefPtr.h>
#include <LibWeb/DOM/Node.h>
#include <LibWeb/Forward.h>

namespace Web::DOM {

class Position {
public:
    Position() { }
    Position(Node&, unsigned offset);

    ~Position();

    bool is_valid() const { return m_node; }

    Node* node() { return m_node; }
    const Node* node() const { return m_node; }

    unsigned offset() const { return m_offset; }
    void set_offset(unsigned value) { m_offset = value; }

    bool operator==(const Position& other) const
    {
        return m_node == other.m_node && m_offset == other.m_offset;
    }

    bool operator!=(const Position& other) const
    {
        return !(*this == other);
    }

private:
    RefPtr<Node> m_node;
    unsigned m_offset { 0 };
};

class Range : public RefCounted<Range> {
public:
    Range(Document&);
    Range(Node& start, size_t start_offset, Node& end, size_t end_offset)
        : m_start(start)
        , m_end(end)
        , m_start_offset(start_offset)
        , m_end_offset(end_offset)
    {
    }

    NonnullRefPtr<Node> start() const { return m_start; }
    void set_start(Node& node, size_t offset)
    {
        m_start = node;
        m_start_offset = offset;
    }

    NonnullRefPtr<Node> end() const { return m_end; }
    void set_end(Node& node, size_t offset)
    {
        m_end = node;
        m_end_offset = offset;
    }

    size_t start_offset() const { return m_start_offset; }
    size_t end_offset() const { return m_end_offset; }

    bool collapsed() const { return m_start.ptr() == m_end.ptr() && m_start_offset == m_end_offset; }

    NonnullRefPtr<Range> normalized() const;

private:
    NonnullRefPtr<Node> m_start;
    NonnullRefPtr<Node> m_end;

    size_t m_start_offset;
    size_t m_end_offset;
};

const LogStream&
operator<<(const LogStream&, const Position&);

}
