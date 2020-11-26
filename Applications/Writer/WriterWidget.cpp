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

#include <Applications/Writer/MainWindowUI.h>
#include <Applications/Writer/WriterWidget.h>
#include <LibWeb/HTML/HTMLElement.h>
#include <LibWeb/InProcessWebView.h>

namespace Writer {

constexpr const char* html_document_template = R"~~~(
<html>
    <head>
    </head>
    <body>
    </body>
</html>
)~~~";

REGISTER_WIDGET(Writer, WriterWidget)

WriterWidget::WriterWidget()
{
    load_from_json(main_window_ui_json);

    m_web_view = static_cast<Web::InProcessWebView&>(*find_descendant_by_name("web_view"));
}

void WriterWidget::create_document()
{
    if (m_document)
        remove_child(*m_document);

    m_web_view->load_html(html_document_template, "application://writer");

    // FIXME: m_web_view->document()->body() returns a const element for some reason?
    m_document = Node::construct(*m_web_view->document(), nullptr, const_cast<Web::HTML::HTMLElement*>(m_web_view->document()->body()));
    add_child(*m_document);
}

}
