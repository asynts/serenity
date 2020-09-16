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

#include <AK/LexicalPath.h>
#include <LibCore/ArgsParser.h>
#include <LibCore/ProgramPathIterator.h>

#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if (pledge("stdio exec rpath", nullptr) < 0) {
        perror("pledge");
        exit(1);
    }

    bool ignore_environment { false };
    Vector<const char*> arguments;

    Core::ArgsParser args_parser;
    args_parser.add_option(ignore_environment, "Ignore environment", "ignore-environment", 'i');
    args_parser.add_positional_argument(arguments, "Arguments", "arguments", Core::ArgsParser::Required::No);
    args_parser.parse(argc, argv);

    const char* exec_filename = nullptr;
    Vector<char*> exec_argv;
    Vector<char*> exec_environ;

    bool parse_environ = true;
    for (auto argument : arguments) {
        if (parse_environ && StringView { argument }.contains('=')) {
            exec_environ.append(strdup(argument));
        } else if (exec_filename == nullptr) {
            exec_filename = argument;
            parse_environ = false;
        } else {
            exec_argv.append(strdup(argument));
        }
    }

    if (ignore_environment)
        clearenv();

    for (auto env : exec_environ)
        putenv(env);

    if (exec_filename == nullptr) {
        for (auto env = environ; *env != nullptr; ++env)
            out() << *env;

        exit(0);
    } else {
        for (Core::ProgramPathIterator programs; programs.has_next();) {
            auto program = programs.next_executable();
            auto basename = LexicalPath { program }.basename();

            if (basename == exec_filename) {
                exec_argv.prepend(strdup(basename.characters()));
                exec_argv.append(nullptr);

                execv(program.characters(), exec_argv.data());

                perror("environ");
                exit(1);
            }
        }

        warn() << "No such file or directory";
        exit(1);
    }
}
