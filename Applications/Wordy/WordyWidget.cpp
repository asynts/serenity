#include <LibGUI/Painter.h>
#include <LibWeb/Painting/PaintContext.h>
#include <LibGUI/ScrollBar.h>
#include <LibWeb/Layout/LayoutDocument.h>
#include <LibWeb/DOM/Element.h>
#include <LibWeb/Page/Frame.h>

#include "WordyWidget.h"

namespace Wordy {
    WordyWidget::WordyWidget()
        : m_page(*this)
    {
        m_document = Web::DOM::Document::create("memory://wordy");

        auto p_element = m_document->create_element("p");
        p_element->set_inner_text("Hello, World!");
        m_document->append_child(p_element);

        m_frame = Web::Frame::create(m_page);
        m_frame->set_document(m_document);
    }

    void WordyWidget::paint_event(GUI::PaintEvent& event)
    {
        GUI::ScrollableWidget::paint_event(event);

        GUI::Painter painter{*this};
        Web::PaintContext context{painter, palette(), { horizontal_scrollbar().value(), vertical_scrollbar().value() }};

        // FIXME: When is this LayoutDocument created?
        if (m_document->layout_node())
            m_document->layout_node()->paint_all_phases(context);
    }
}
