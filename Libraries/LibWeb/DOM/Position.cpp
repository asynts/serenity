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

#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/Node.h>
#include <LibWeb/DOM/Position.h>

namespace Web::DOM {

Position::Position(Node& node, unsigned offset)
    : m_node(node)
    , m_offset(offset)
{
}

Position::~Position()
{
}

Range::Range(Document& document)
    : m_start(document)
    , m_end(document)
    , m_start_offset(0)
    , m_end_offset(0)
{
}

NonnullRefPtr<Range> Range::normalized() const
{
    if (m_start.ptr() == m_end.ptr()) {
        if (m_start_offset <= m_end_offset)
            return *this;

        return adopt(*new Range(const_cast<Node&>(*m_end), m_end_offset, const_cast<Node&>(*m_start), m_start_offset));
    }

    if (m_start->is_before(m_end))
        return adopt(*new Range(const_cast<Node&>(*m_start), m_end_offset, const_cast<Node&>(*m_end), m_start_offset));

    return adopt(*new Range(const_cast<Node&>(*m_end), m_end_offset, const_cast<Node&>(*m_start), m_start_offset));
}

const LogStream& operator<<(const LogStream& stream, const Position& position)
{
    if (!position.node())
        return stream << "DOM::Position(nullptr, " << position.offset() << ")";
    return stream << "DOM::Position(" << position.node()->node_name() << "{" << position.node() << "}, " << position.offset() << ")";
}

}
