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
#include <LibWeb/DOM/Range.h>
#include <LibWeb/DOM/Text.h>
#include <LibWeb/Layout/LayoutPosition.h>
#include <LibWeb/Page/Frame.h>

#include <LibWeb/DOM/Document.h>
#include <LibWeb/Dump.h>
#include <LibWeb/Layout/InitialContainingBlockBox.h>

#include "EditEventHandler.h"

namespace Web {

void EditEventHandler::on_select(DOM::Range& range)
{
    m_dom_selection = range;
}

void EditEventHandler::on_delete_pressed()
{
    if (!dom_selection())
        return;

    if (dom_selection()->is_collapsed())
        on_navigate(Direction::Right, true);

    delete_dom_range(*dom_selection());
    m_dom_selection.clear();
}

void EditEventHandler::on_backspace_pressed()
{
    if (!dom_selection())
        return;

    if (dom_selection()->is_collapsed())
        on_navigate(Direction::Left, true);

    delete_dom_range(*dom_selection());
    m_dom_selection.clear();
}

void EditEventHandler::on_insert(StringView)
{
    if (!dom_selection())
        return;

    if (!dom_selection()->is_collapsed()) {
        delete_dom_range(*dom_selection());
        m_dom_selection = m_dom_selection->start();
    }
}

void EditEventHandler::on_navigate(Direction, bool)
{
    TODO();
}

void EditEventHandler::delete_dom_range(const DOM::Range&)
{
    TODO();
}

}
