#include "ParagraphView.h"

#include <LibGUI/Layout.h>
#include <LibGUI/BoxLayout.h>
#include <LibGUI/Label.h>
#include <LibGfx/Font.h>

namespace Wordy {
    ParagraphView::ParagraphView(ParagraphNode& paragraph)
        : m_paragraph(paragraph)
    {
        set_layout<GUI::VerticalBoxLayout>().set_margins({ 2, 2, 2, 2 });

        for (auto& snippet : m_paragraph->snippets()) {
            auto& label = add<GUI::Label>(snippet->contents());

            if (snippet->bold())
                label.set_font(Gfx::Font::default_bold_font());
        }
    }
}
