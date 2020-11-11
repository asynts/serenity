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

#include <LibGUI/Application.h>
#include <LibGUI/Window.h>

#include "WordyWidget.h"

int main(int argc, char** argv) {
    auto app = GUI::Application::construct(argc, argv);

    auto document = Wordy::Document::construct();

    auto paragraph1 = Wordy::ParagraphNode::construct();
    paragraph1->snippets().append(Wordy::TextNode::construct("Hello, "));
    paragraph1->snippets().append(Wordy::TextNode::construct("World", true));
    paragraph1->snippets().append(Wordy::TextNode::construct("!"));
    document->paragraphs().append(paragraph1);

    auto paragraph2 = Wordy::ParagraphNode::construct();
    paragraph2->snippets().append(Wordy::TextNode::construct("This is another paragraph!"));
    document->paragraphs().append(paragraph2);

    auto window = GUI::Window::construct();
    window->set_title("Wordy");
    window->resize(640, 480);
    window->set_main_widget<Wordy::WordyWidget>(*document);
    window->show();

    return app->exec();
}
