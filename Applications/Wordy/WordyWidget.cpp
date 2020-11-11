#include "WordyWidget.h"

#include <LibGUI/Label.h>
#include <LibGUI/Layout.h>
#include <LibGUI/BoxLayout.h>

namespace Wordy {
    WordyWidget::WordyWidget(Document& document)
        : m_document(document)
    {
        set_fill_with_background_color(true);
        set_layout<GUI::VerticalBoxLayout>();
        layout()->set_margins({ 2, 2, 2, 2 });

        add<GUI::Label>(m_document->contents());
    }
}
