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

    bool has_next()
    {
        if (m_next_program.has_value())
            return true;

        if (m_dir_iterator.has_value()) {
            if (m_dir_iterator.value().has_next()) {
                m_next_program = m_dir_iterator.value().next_full_path();

                if (access(m_next_program.value().characters(), X_OK) == 0)
                    return true;

                m_next_program.clear();
                return has_next();
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

    String next_program()
    {
        has_next();

        auto program = m_next_program.value();
        m_next_program.clear();

        return program;
    }

private:
    Vector<StringView> m_directories;
    Optional<DirIterator> m_dir_iterator;
    Optional<String> m_next_program;
};

}
