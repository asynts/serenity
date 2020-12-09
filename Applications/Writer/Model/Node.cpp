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
#include <LibWeb/DOM/Element.h>
#include <LibWeb/HTML/HTMLElement.h>

#include <Applications/Writer/Model/Node.h>

namespace Writer {

void Node::replace_element_with(Web::DOM::Element& new_element)
{
    if (element())
        element()->replace_with(new_element);
    else
        parent()->element()->append_child(new_element);

    m_element = new_element;
}

DocumentNode::DocumentNode(Web::DOM::Document& document)
    : Node(document)
{
    set_element(const_cast<Web::HTML::HTMLElement&>(*document.body()));
}

void DocumentNode::render()
{
    for_each_child([&](Node& node) {
        node.render();
    });
}

void ParagraphNode::render()
{
    auto new_element = document().create_element("p");

    replace_element_with(new_element);

    for_each_child([&](Node& node) {
        node.render();
    });
}

void FragmentNode::render()
{
    auto new_element = document().create_element("span");

    new_element->set_text_content(m_content);

    // FIXME: Add 'bold' if needed.

    replace_element_with(new_element);
}

}
