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

#include <Applications/Writer/InspectorWidget.h>
#include <Applications/Writer/WriterWidget.h>
#include <LibCore/ArgsParser.h>
#include <LibGUI/Action.h>
#include <LibGUI/Menu.h>
#include <LibGUI/MenuBar.h>
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

    auto& writer = window->set_main_widget<Writer::WriterWidget>();

    writer.create_document();
    writer.document()->load_from_json(input_file);

    RefPtr<GUI::Window> inspector_window;

    auto inspect_dom_tree_action = GUI::Action::create("Inspect DOM tree", { Mod_None, Key_F12 }, [&](auto&) {
        if (!inspector_window) {
            inspector_window = GUI::Window::construct();
            inspector_window->resize(300, 500);
            inspector_window->set_title("DOM inspector");
            inspector_window->set_icon(Gfx::Bitmap::load_from_file("/res/icons/16x16/inspector-object.png"));
        }

        auto& inspector = inspector_window->set_main_widget<Writer::InspectorWidget>();
        inspector.set_document(*writer.web_view()->document());

        inspector_window->show();
    });

    auto menubar = GUI::MenuBar::construct();

    auto& inspect_menu = menubar->add_menu("Inspect");
    inspect_menu.add_action(*inspect_dom_tree_action);

    app->set_menubar(menubar);

    window->show();

    return app->exec();
}
