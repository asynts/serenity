#include <LibGUI/Application.h>
#include <LibGUI/Icon.h>
#include <LibGUI/Widget.h>
#include <LibGUI/Window.h>

#include <Applications/Writer/WriterWidget.h>

int main(int argc, char** argv)
{
    auto app = GUI::Application::construct(argc, argv);

    auto app_icon = GUI::Icon::default_icon("app-writer");

    auto window = GUI::Window::construct();
    window->set_title("Writer");

    window->set_main_widget<Writer::WriterWidget>();

    window->show();
    window->set_icon(app_icon.bitmap_for_size(16));

    return app->exec();
}
