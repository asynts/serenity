#include "DocumentView.h"

#include <LibGUI/Layout.h>
#include <LibGUI/BoxLayout.h>

namespace Wordy {
    DocumentView::DocumentView(Document& document)
        : m_document(document)
    {
        set_layout<GUI::VerticalBoxLayout>().set_margins({ 2, 2, 2, 2 });

        m_contents_label = add<GUI::Label>(document.contents());
    }
}
