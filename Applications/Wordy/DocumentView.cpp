#include "DocumentView.h"
#include "ParagraphView.h"

#include <LibGUI/Layout.h>
#include <LibGUI/BoxLayout.h>

namespace Wordy {
    DocumentView::DocumentView(Document& document)
        : m_document(document)
    {
        set_layout<GUI::VerticalBoxLayout>().set_margins({ 2, 2, 2, 2 });

        for(auto& paragraph : m_document->paragraphs())
            add<ParagraphView>(*paragraph);
    }
}
