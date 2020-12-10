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

#include <LibWeb/InProcessWebView.h>

#include <Applications/Writer/WriterWidget.h>
#include <Applications/Writer/WriterWidgetUI.h>

namespace Writer {

constexpr const char* html_template = R"~~~(
<html>
    <head>
        <style>
            .bold { font-weight: bold; }
        </style>
    </head>
    <body></body>
</html>
)~~~";

WriterWidget::WriterWidget()
{
    load_from_json(writer_widget_ui_json);

    m_webview = static_cast<Web::InProcessWebView*>(find_descendant_by_name("webview"));
    m_webview->load_html(html_template, "memory://writer");

    m_document = DocumentNode::create(*m_webview->document());

    auto paragraph = m_document->create_child<ParagraphNode>();

    auto fragment1 = paragraph->create_child<FragmentNode>();
    fragment1->set_content("Hello, ");

    auto fragment2 = paragraph->create_child<FragmentNode>();
    fragment2->set_content("world");
    fragment2->set_bold(true);

    auto fragment3 = paragraph->create_child<FragmentNode>();
    fragment3->set_content("!");

    m_document->render();
}

}
