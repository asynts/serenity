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
#include <LibWeb/DOM/Range.h>
#include <LibWeb/DOM/Text.h>
#include <LibWeb/Layout/LayoutPosition.h>
#include <LibWeb/Page/Frame.h>

#include <LibWeb/DOM/Document.h>
#include <LibWeb/Dump.h>
#include <LibWeb/Layout/InitialContainingBlockBox.h>

#include "EditEventHandler.h"

namespace Web {

void EditEventHandler::on_select(DOM::Range range)
{
    m_selection = range;

    if (m_frame.document() && m_frame.document()->layout_node())
        m_frame.document()->layout_node()->recompute_selection_states();
}

bool EditEventHandler::on_backspace_pressed()
{
    if (!selection())
        return false;

    auto cached_selection = selection();
    auto cached_cursor = cursor();

    if (cached_selection->is_collapsed()) {
        if (cached_cursor->offset() == 0)
            TODO();

        m_selection = DOM::Position { cached_cursor->node(), cached_cursor->offset() - 1 };

        delete_dom_range({
            { cached_cursor->node(), cached_cursor->offset() - 1 },
            { cached_cursor->node(), cached_cursor->offset() },
        });
    } else {
        m_selection = cached_selection->normalized().start();

        delete_dom_range(*cached_selection);
    }

    update_dom();
    return true;
}

bool EditEventHandler::on_delete_pressed()
{
    if (!selection())
        return false;

    auto old_selection = selection();
    auto old_cursor = cursor();

    if (old_selection->is_collapsed()) {
        auto& text = downcast<DOM::Text>(old_cursor->node());

        if (old_cursor->offset() >= text.length())
            TODO();

        delete_dom_range({
            { old_cursor->node(), old_cursor->offset() },
            { old_cursor->node(), old_cursor->offset() + 1 },
        });
    } else {
        m_selection = old_selection->normalized().start();

        delete_dom_range(*old_selection);
    }

    update_dom();
    return true;
}

bool EditEventHandler::on_text_inserted(StringView snippet)
{
    if (!selection())
        return false;

    if (selection()->is_collapsed()) {
        auto old_selection = m_selection;
        m_selection = old_selection->normalized().start();

        delete_dom_range(*old_selection);
    }

    auto& text = downcast<DOM::Text>(cursor()->node());

    StringBuilder builder;
    builder.append(text.data().substring_view(0, cursor()->offset()));
    builder.append(snippet);
    builder.append(text.data().substring_view(cursor()->offset()));
    text.set_data(builder.string_view());

    m_selection = DOM::Position { cursor()->node(), cursor()->offset() + snippet.length() };

    update_dom();
    return true;
}

bool EditEventHandler::on_navigation(Direction direction, bool do_select)
{
    if (!selection())
        return false;

    Optional<DOM::Position> new_cursor_position;
    if (direction == Direction::Left) {
        if (cursor()->offset() == 0)
            TODO();

        new_cursor_position = DOM::Position { cursor()->node(), cursor()->offset() - 1 };
    } else if (direction == Direction::Right) {
        auto& text = downcast<DOM::Text>(cursor()->node());
        if (cursor()->offset() >= text.length())
            TODO();

        new_cursor_position = DOM::Position { cursor()->node(), cursor()->offset() + 1 };
    } else {
        TODO();
    }

    if (do_select)
        m_selection = DOM::Range { m_selection->start(), *new_cursor_position };
    else
        m_selection = *new_cursor_position;

    m_frame.blink_cursor(false);
    return true;
}

void EditEventHandler::delete_dom_range(DOM::Range range)
{
    range = range.normalized();

    auto& start = downcast<DOM::Text>(range.start().node());
    auto& end = downcast<DOM::Text>(range.end().node());

    if (&start == &end) {
        StringBuilder builder;
        builder.append(start.data().substring_view(0, range.start().offset()));
        builder.append(end.data().substring_view(range.end().offset()));

        start.set_data(builder.string_view());
    } else {
        // Remove all the nodes that are fully enclosed in the range.
        HashTable<DOM::Node*> queued_for_deletion;
        for (auto* node = start.next_in_pre_order(); node; node = node->next_in_pre_order()) {
            if (node == &end)
                break;

            queued_for_deletion.set(node);
        }
        for (auto* parent = start.parent(); parent; parent = parent->parent())
            queued_for_deletion.remove(parent);
        for (auto* parent = end.parent(); parent; parent = parent->parent())
            queued_for_deletion.remove(parent);
        for (auto* node : queued_for_deletion)
            node->parent()->remove_child(*node);

        // Join the parent nodes of start and end.
        DOM::Node *insert_after = &start, *remove_from = &end, *parent_of_end = end.parent();
        while (remove_from) {
            auto* next_sibling = remove_from->next_sibling();

            remove_from->parent()->remove_child(*remove_from);
            insert_after->parent()->insert_before(*remove_from, *insert_after);

            insert_after = remove_from;
            remove_from = next_sibling;
        }
        if (!parent_of_end->has_children()) {
            if (parent_of_end->parent())
                parent_of_end->parent()->remove_child(*parent_of_end);
        }

        // Join the start and end nodes.
        StringBuilder builder;
        builder.append(start.data().substring_view(0, range.start().offset()));
        builder.append(end.data().substring_view(range.end().offset()));

        start.set_data(builder.string_view());
        start.parent()->remove_child(end);
    }
}

void EditEventHandler::update_dom()
{
    // FIXME: When nodes are removed from the DOM, the associated layout nodes become stale and still
    //        remain in the layout tree. This has to be fixed, this just causes everything to be recomputed
    //        which really hurts performance.
    m_frame.document()->force_layout();
}

}
