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

#include <LibWeb/DOM/AbstractRange.h>

namespace Web::DOM {

class Range final : public AbstractRange {
public:
    Node* start_container() override { return m_start_container; }
    unsigned start_offset() override { return m_start_offset; }

    Node* end_container() override { return m_end_container; }
    unsigned end_offset() override { return m_end_offset; }

    void set_start(Node* container, unsigned offset)
    {
        m_start_container = container;
        m_start_offset = offset;
    }

    void set_end(Node* container, unsigned offset)
    {
        m_end_container = container;
        m_end_offset = offset;
    }

    void set_start_before(Node* node) { TODO(); }
    void set_start_after(Node* node) { TODO(); }
    void set_end_before(Node* node) { TODO(); }
    void set_end_after(Node* node) { TODO(); }
    void collapse(bool to_start = false) { TODO(); }
    void select_node(Node* node) { TODO(); }
    void select_node_contents(Node* node) { TODO(); }
    short compare_boundary_points(unsigned short how, Range sourceRange) { TODO(); }
    void delete_contents() { TODO(); }
    DocumentFragment* extract_contents() { TODO(); }
    DocumentFragment* clone_contents() { TODO(); }
    void insert_node(Node* node) { TODO(); }
    void surround_contents(Node* newParent) { TODO(); }
    Range* clone_range() { TODO(); }
    void detach() { TODO(); }
    bool is_point_in_range(Node* node, unsigned long offset) { TODO(); }
    short compare_point(Node* node, unsigned long offset) { TODO(); }
    bool intersects_node(Node* node);

private:
    Node* m_start_container;
    unsigned m_start_offset;

    Node* m_end_container;
    unsigned m_end_offset;
};

}
