#include <LibCore/ArgsParser.h>
#include <LibGUI/Application.h>
#include <LibGUI/Icon.h>
#include <LibGUI/Widget.h>
#include <LibGUI/Window.h>

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

    auto& writer = window->set_main_widget<Writer::WriterWidget>();

    if (file_to_edit) {
        bool success = writer.open_file(file_to_edit);
        ASSERT(success);
    }

    window->show();
    window->set_icon(app_icon.bitmap_for_size(16));

    return app->exec();
}
