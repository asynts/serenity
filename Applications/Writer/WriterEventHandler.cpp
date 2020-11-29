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

#include <Applications/Writer/WriterEventHandler.h>
#include <LibGUI/Event.h>
#include <LibWeb/Layout/InitialContainingBlockBox.h>
#include <LibWeb/Layout/Node.h>
#include <LibWeb/Page/Frame.h>

namespace Writer {

bool WriterEventHandler::handle_keydown(KeyCode key, unsigned modifiers, u32 code_point)
{
    return true;
}

bool WriterEventHandler::handle_mousedown(const Gfx::IntPoint& position, unsigned button, unsigned modifiers)
{
    if (button == GUI::MouseButton::Left) {
        auto result = layout_root()->hit_test(position, Layout::HitTestType::TextCursor);

        if (result.layout_node && result.layout_node->dom_node()) {
            m_frame.set_cursor_position(DOM::Position(*node, result.index_in_node));
            layout_root()->set_selection({ { result.layout_node, result.index_in_node }, {} });
            dump_selection("MouseDown");
            m_in_mouse_selection = true;
        }
    }

    return EventHandler::handle_mousedown(position, button, modifiers);
}

}
