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

#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/Position.h>
#include <LibWeb/DOM/Range.h>

#include <Applications/Writer/EditEventHandler.h>

namespace Writer {

EditEventHandler::EditEventHandler(Writer::DocumentNode& document)
    : Web::EditEventHandler(*document.dom().frame())
    , m_document(document)
{
}

void EditEventHandler::handle_delete(Web::DOM::Range& range)
{
    dbgln("{}", __PRETTY_FUNCTION__);

    auto* start_node = m_document.lookup(*range.start_container());
    dbgln("deletion starts in {}+{} ({})", start_node, range.start_offset(), start_node->class_name());

    auto* end_node = m_document.lookup(*range.start_container());
    dbgln("deletion ends in {}+{} ({})", end_node, range.end_offset(), end_node->class_name());
}

void EditEventHandler::handle_insert(Web::DOM::Position, u32)
{
    dbgln("{}", __PRETTY_FUNCTION__);
}

}
