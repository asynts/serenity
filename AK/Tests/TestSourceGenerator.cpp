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

#include <AK/TestSuite.h>

#include <AK/SourceGenerator.h>

TEST_CASE(generate_c_code)
{
    SourceGenerator::MappingType mappings;
    mappings.set("name", "foo");

    SourceGenerator generator { "const char* @name@ (void) { return \"@name@\"; }", mappings };

    EXPECT_EQ(generator.generate(), "const char* foo (void) { return \"foo\"; }");
}

TEST_CASE(override_mappings)
{
    SourceGenerator::MappingType mappings;
    mappings.set("foo", "13");
    mappings.set("bar", "23");

    SourceGenerator::MappingType override_mappings;
    mappings.set("bar", "42");

    SourceGenerator generator { "@foo@ @bar@", mappings };

    EXPECT_EQ(generator.generate(&override_mappings), "13 42");
}

TEST_CASE(copy_mappings)
{
    SourceGenerator::MappingType mappings;
    mappings.set("foo", "13");
    mappings.set("bar", "23");

    SourceGenerator generator { "@foo@ @bar@", mappings };

    mappings.set("bar", "42");

    EXPECT_EQ(generator.generate(), "13 23");
}

TEST_MAIN(SourceGenerator)
