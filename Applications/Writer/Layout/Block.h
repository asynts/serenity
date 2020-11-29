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

#pragma once

#include <AK/String.h>
#include <LibGfx/Font.h>
#include <LibGfx/Painter.h>
#include <LibGfx/Point.h>
#include <LibGfx/Rect.h>
#include <LibGfx/Size.h>
#include <LibWeb/TreeNode.h>

namespace Writer::Layout {

class Block : public Web::TreeNode<Block> {
public:
    virtual ~Block() = default;

    void inserted_into(Block&) { }
    void children_changed() { }

    virtual Gfx::IntSize size() const = 0;
    virtual void draw(Gfx::Painter&, Gfx::IntPoint) = 0;
};

class HorizontalBlock final : public Block {
public:
    Gfx::IntSize size() const override
    {
        Gfx::IntSize total;
        for_each_child([&](const Block& block) {
            total.set_width(total.width() + block.size().width());
            return IterationDecision::Continue;
        });

        return total;
    }

    void draw(Gfx::Painter& painter, Gfx::IntPoint point) override
    {
        for_each_child([&](Block& block) {
            block.draw(painter, point);
            point.set_x(point.x() + block.size().width());
        });
    }
};

class VerticalBlock final : public Block {
public:
    using Block::Block;

    Gfx::IntSize size() const override
    {
        Gfx::IntSize total;
        for_each_child([&](const Block& block) {
            total.set_height(total.height() + block.size().height());
            return IterationDecision::Continue;
        });

        return total;
    }

    void draw(Gfx::Painter& painter, Gfx::IntPoint point) override
    {
        for_each_child([&](Block& block) {
            block.draw(painter, point);
            point.set_y(point.y() + block.size().height());
        });
    }
};

class TextBlock final : public Block {
public:
    TextBlock(Gfx::Font& font, String text)
        : m_text(text)
        , m_font(font)
    {
    }

    String text() const { return m_text; }
    void set_text(String value) { m_text = value; }

    Gfx::IntSize size() const override
    {
        return { m_font.width(m_text), (int)m_font.glyph_height() };
    }

    void draw(Gfx::Painter& painter, Gfx::IntPoint point) override
    {
        painter.draw_text({ point, size() }, text(), m_font);
    }

private:
    String m_text;
    Gfx::Font& m_font;
};

}
