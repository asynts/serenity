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
#include <LibWeb/Bindings/Wrappable.h>
#include <LibWeb/DOM/Range.h>

namespace Web::DOM {

// This API is soo weird. JavaScript...
class Selection
    : public RefCounted<Selection>
    , public Bindings::Wrappable {
public:
    using WrapperType = Bindings::SelectionWrapper;

    static NonnullRefPtr<Selection> create()
    {
        return adopt(*new Selection);
    }

    static NonnullRefPtr<Selection> create(Range& range)
    {
        return adopt(*new Selection(range));
    }

    Node* anchor_node()
    {
        if (!m_range)
            return nullptr;

        return m_range->end_container();
    }

    unsigned anchor_offset()
    {
        if (!m_range)
            return 0;

        return m_range->end_offset();
    }

    Node* focus_node()
    {
        if (!m_range)
            return nullptr;

        return m_range->start_container();
    }

    unsigned focus_offset()
    {
        if (!m_range)
            return 0;

        return m_range->start_offset();
    }

    bool is_collapsed()
    {
        return anchor_node() != focus_node() && anchor_offset() == focus_offset();
    }

    unsigned range_count() { return (bool)m_range; }

    String type()
    {
        if (m_range) {
            if (is_collapsed())
                return "Caret";

            return "Range";
        }

        return "None";
    }

    Range* get_range_at(unsigned index)
    {
        ASSERT(index == 0);
        return m_range;
    }

    void add_range(Range& range)
    {
        if (!m_range)
            m_range = range;
    }

    bool remove_range(Range& range)
    {
        if (&range != m_range.ptr())
            return false;

        m_range.clear();
        return true;
    }

    void remove_all_ranges()
    {
        m_range.clear();
    }

    // FIXME: There are a ton of methods missing here.

private:
    Selection() = default;

    explicit Selection(Range& range)
    {
        m_range = range;
    }

    RefPtr<Range> m_range;
};

}
