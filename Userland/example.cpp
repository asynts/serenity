#include <LibGUI/Widget.h>

int main()
{
    GUI::WidgetClassRegistration::for_each([](const auto& registration) {
        dbgln("{}", registration.class_name());
    });
}
