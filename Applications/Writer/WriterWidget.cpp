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

#include <LibWeb/HTML/HTMLElement.h>
#include <LibWeb/InProcessWebView.h>
#include <LibWeb/Page/EventHandler.h>
#include <LibWeb/Page/Frame.h>

#include <Applications/Writer/EditEventHandler.h>
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
    <body contenteditable></body>
</html>
)~~~";

constexpr const char* example_writer_document = R"~~~(
{
    "class": "DocumentNode",
    "children": [
        {
            "class": "ParagraphNode",
            "children": [
                {
                    "class": "FragmentNode",
                    "content": "abc ",
                    "bold": false
                },
                {
                    "class": "FragmentNode",
                    "content": "def",
                    "bold": true
                },
                {
                    "class": "FragmentNode",
                    "content": " ghi",
                    "bold": false
                }
            ]
        },
        {
            "class": "ParagraphNode",
            "children": [
                {
                    "class": "FragmentNode",
                    "content": "jkl",
                    "bold": false
                }
            ]
        },
        {
            "class": "ParagraphNode",
            "children": [
                {
                    "class": "FragmentNode",
                    "content": "mno",
                    "bold": false
                }
            ]
        }
    ]
}
)~~~";

WriterWidget::WriterWidget()
{
    load_from_json(writer_widget_ui_json);

    m_webview = static_cast<Web::InProcessWebView*>(find_descendant_by_name("webview"));
    m_webview->load_html(html_template, "memory://writer");

    replace_document(DocumentNode::create_from_json(*m_webview->document(), example_writer_document));
}

void WriterWidget::replace_document(DocumentNode& document)
{
    auto new_edit_event_handler = make<EditEventHandler>(document);
    m_webview->document()->frame()->event_handler().set_edit_event_handler(move(new_edit_event_handler));

    m_document = document;

    // FIXME: Rename to attach_to_dom?
    document.set_element(*m_webview->document()->body());
    document.render();
}

}
