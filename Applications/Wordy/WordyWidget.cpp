#include "WordyWidget.h"

#include <LibGUI/Label.h>
#include <LibGUI/Layout.h>
#include <LibGUI/BoxLayout.h>

namespace Wordy {
    WordyWidget::WordyWidget(Document& document)
    {
        set_fill_with_background_color(true);
        set_layout<GUI::VerticalBoxLayout>().set_margins({ 2, 2, 2, 2 });

        m_document_view = add<DocumentView>(document);
    }
}
