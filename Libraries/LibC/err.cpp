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

#include <AK/kmalloc.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

__attribute__((noreturn)) void err(int eval, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    verr(eval, fmt, ap);
    va_end(ap);
}

__attribute__((noreturn)) void verr(int eval, const char* fmt, va_list ap)
{
    if (fmt) {
        char* message = nullptr;
        vasprintf(&message, fmt, ap);
        fprintf(stderr, "%s: %s: %s\n", getprogname(), message, strerror(errno));
        free(message);
    } else {
        fprintf(stderr, "%s: %s\n", getprogname(), strerror(errno));
    }

    fprintf(stderr, "%s: %s\n", getprogname(), strerror(errno));
    exit(eval);
}

__attribute__((noreturn)) void errx(int eval, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    verrx(eval, fmt, ap);
    va_end(ap);
}

__attribute__((noreturn)) void verrx(int eval, const char* fmt, va_list ap)
{
    if (fmt) {
        char* message = nullptr;
        vasprintf(&message, fmt, ap);
        fprintf(stderr, "%s: %s\n", getprogname(), message);
        free(message);
    } else {
        fprintf(stderr, "%s: \n", getprogname());
    }

    exit(eval);
}
