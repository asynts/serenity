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

#include <LibCore/ArgsParser.h>
#include <LibGUI/Application.h>
#include <LibGUI/FilePicker.h>
#include <LibGUI/Menu.h>
#include <LibGUI/MenuBar.h>
#include <LibGUI/Window.h>
#include <LibWeb/InProcessWebView.h>

#include <Applications/Writer/Model/DocumentNode.h>
#include <Applications/Writer/WriterWidget.h>

int main(int argc, char** argv)
{
    auto app = GUI::Application::construct(argc, argv);
    auto app_icon = GUI::Icon::default_icon("app-writer");

    const char* file_to_edit = nullptr;

    Core::ArgsParser args_parser;
    args_parser.add_positional_argument(file_to_edit, "File to edit", "file", Core::ArgsParser::Required::No);
    args_parser.parse(argc, argv);

    auto window = GUI::Window::construct();
    window->set_title("Writer");
    window->resize(600, 500);

    auto& writer = window->set_main_widget<Writer::WriterWidget>();

    if (file_to_edit) {
        auto new_document = Writer::DocumentNode::create_from_file(*writer.webview().document(), file_to_edit);
        writer.replace_document(new_document);
    }

    auto app_save_as_action = GUI::CommonActions::make_save_as_action([&](auto&) {
        Optional<String> save_path = GUI::FilePicker::get_save_filepath(window, "Untitled", "writer");
        if (!save_path.has_value())
            return;

        ASSERT(writer.document());
        writer.document()->write_to_file(save_path.value());

        dbgln("Wrote document to {}", save_path.value());
    });

    auto app_open_action = GUI::CommonActions::make_open_action([&](auto&) {
        Optional<String> open_path = GUI::FilePicker::get_open_filepath(window);

        if (!open_path.has_value())
            return;

        auto new_document = Writer::DocumentNode::create_from_file(*writer.webview().document(), open_path.value());
        writer.replace_document(new_document);

        dbgln("Read document from {}", open_path.value());
    });

    auto menubar = GUI::MenuBar::construct();
    auto& app_menu = menubar->add_menu("Writer");
    app_menu.add_action(app_save_as_action);
    app_menu.add_action(app_open_action);

    app->set_menubar(menubar);

    window->set_icon(app_icon.bitmap_for_size(16));
    window->show();

    return app->exec();
}
