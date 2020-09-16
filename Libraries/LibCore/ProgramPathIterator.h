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

#include <AK/Optional.h>
#include <AK/StringView.h>
#include <AK/Vector.h>
#include <LibCore/DirIterator.h>

#include <stdlib.h>

namespace Core {

class ProgramPathIterator {
public:
    ProgramPathIterator()
    {
        m_directories = StringView { getenv("PATH") }.split_view(':');
    }

    bool has_next() const
    {
        if (m_dir_iterator.has_value()) {
            if (m_dir_iterator.value().has_next()) {
                return true;
            } else {
                m_dir_iterator.clear();
                return has_next();
            }
        }

        if (m_directories.size() > 0) {
            m_dir_iterator.emplace(m_directories.take_first(), DirIterator::Flags::SkipParentAndBaseDir);
            return has_next();
        }

        return false;
    }

    String next_program() { return m_dir_iterator.value().next_full_path(); }

private:
    mutable Vector<StringView> m_directories;
    mutable Optional<DirIterator> m_dir_iterator;
};

}
