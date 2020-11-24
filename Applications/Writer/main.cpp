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
#include <LibCore/ArgsParser.h>
#include <LibWeb/InProcessWebView.h>

const char input_file[] = R"~~~(
{
    "children": [
        {
            "class": "ParagraphNode",
            "children": [
                { "class": "FragmentNode", "content": "Hello, " },
                { "class": "FragmentNode", "content": "Paul", "bold": true },
                { "class": "FragmentNode", "content": "!" }
            ]
        }
    ]
}
)~~~";

int main(int argc, char** argv)
{
    auto app = GUI::Application::construct(argc, argv);

    auto window = GUI::Window::construct();
    window->set_title("Writer");
    window->resize(570, 500);

    auto& widget = window->set_main_widget<GUI::Widget>();
    dbgln("{}", main_window_ui_json);
    widget.load_from_json(main_window_ui_json);

    auto& writer = static_cast<Writer::WriterWidget&>(*widget.find_descendant_by_name("writer"));

    writer.create_document();
    writer.document()->load_from_json(input_file);

    window->show();

    return app->exec();
}
