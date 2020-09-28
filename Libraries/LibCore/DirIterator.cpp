/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
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

#include <AK/Function.h>
#include <LibCore/DirIterator.h>
#include <dirent.h>
#include <errno.h>

namespace Core {

int iterate_directory(String dirpath, DirectoryIterationFlags flags, Function<IterationDecision(String)> callback)
{
    auto* dir = opendir(dirpath.characters());
    if (dir == nullptr)
        return errno;

    String path;
    do {
        const auto* entry = readdir(dir);
        if (entry == nullptr) {
            const auto error = errno;
            closedir(dir);
            return error;
        }

        path = entry->d_name;

        if (flags & DirectoryIterationFlags::SkipDots && path.starts_with('.'))
            continue;
        if (flags & DirectoryIterationFlags::SkipParentAndBaseDir && (path == "." || path == ".."))
            continue;

        if (flags & DirectoryIterationFlags::FullPath)
            path = String::formatted("{}/{}", dirpath, path);
    } while (callback(path) == IterationDecision::Continue);

    closedir(dir);
    return 0;
}

String find_executable_in_path(String filename)
{
    if (filename.starts_with('/')) {
        if (access(filename.characters(), X_OK) == 0)
            return filename;

        return {};
    }

    for (auto directory : String { getenv("PATH") }.split(':')) {
        auto fullpath = String::format("%s/%s", directory.characters(), filename.characters());

        if (access(fullpath.characters(), X_OK) == 0)
            return fullpath;
    }

    return {};
}

}
