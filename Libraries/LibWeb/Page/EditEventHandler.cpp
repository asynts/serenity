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

#include <AK/StringBuilder.h>
#include <LibWeb/DOM/Position.h>
#include <LibWeb/DOM/Text.h>
#include <LibWeb/Layout/LayoutPosition.h>
#include <LibWeb/Page/Frame.h>

#include <LibWeb/DOM/Document.h>
#include <LibWeb/Dump.h>
#include <LibWeb/Layout/InitialContainingBlockBox.h>

#include "EditEventHandler.h"

namespace Web {

void EditEventHandler::handle_delete(DOM::Range range)
{
    auto* start = downcast<DOM::Text>(range.start().node());
    auto* end = downcast<DOM::Text>(range.end().node());

    if (start->is_before(*end)) {
        // First we remove all the nodes in the "middle" which are entriely covered by the selection.
        DOM::Node* next_in_pre_order;
        for (auto* node = start->next_sibling(); node != end; node = next_in_pre_order) {
            next_in_pre_order = node->next_in_pre_order();
            node->parent()->remove_child(node);
        }

        if (start->next_sibling() == end) {
            // If the start and end text nodes are now immediate siblings, merge the remainders into one.

            StringBuilder builder;
            builder.append(start->data().substring_view(0, range.start().offset()));
            builder.append(end->data().substring_view(range.end().offset()));

            start->set_data(builder.to_string());
            start->parent()->remove_child(end);
        } else {
            // Otherwise, remove parts from both nodes.

            start->set_data(start->data().substring_view(0, range.start().offset()));
            end->set_data(end->data().substring_view(range.end().offset()));
        }

        start->invalidate_style();
        end->invalidate_style();
    } else {
        TODO();
    }

    // FIXME: When nodes are removed from the DOM, the associated layout nodes become stale and still
    //        remain in the layout tree. This has to be fixed, this just causes everything to be recomputed
    //        which really hurts performance.
    m_frame.document()->force_layout();
}

void EditEventHandler::handle_insert(DOM::Position position, u32 code_point)
{
    if (is<DOM::Text>(*position.node())) {
        auto& node = downcast<DOM::Text>(*position.node());

        StringBuilder builder;
        builder.append(node.data().substring_view(0, position.offset()));
        builder.append_code_point(code_point);
        builder.append(node.data().substring_view(position.offset()));
        node.set_data(builder.to_string());

        node.invalidate_style();
    }

    // FIXME: When nodes are removed from the DOM, the associated layout nodes become stale and still
    //        remain in the layout tree. This has to be fixed, this just causes everything to be recomputed
    //        which really hurts performance.
    m_frame.document()->force_layout();
}

}
