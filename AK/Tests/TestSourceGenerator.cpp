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
    SourceGenerator generator;
    generator.set("name", "foo");

    generator.append_pattern("const char* @name@ (void) { return \"@name@\"; }");

    EXPECT_EQ(generator.generate(), "const char* foo (void) { return \"foo\"; }");
}

TEST_CASE(override_mapping)
{
    SourceGenerator generator;
    generator.set("foo", "13");
    generator.set("bar", "23");

    generator.append_pattern("@foo@ @bar@");

    SourceGenerator::MappingType override_mapping;
    override_mapping.set("bar", "42");

    generator.append_pattern(" @foo@ @bar@", &override_mapping);

    EXPECT_EQ(generator.generate(), "13 23 13 42");
}

TEST_CASE(copy_mapping)
{
    SourceGenerator::MappingType mapping;
    mapping.set("foo", "13");
    mapping.set("bar", "23");

    SourceGenerator generator { mapping };

    generator.append_pattern("@foo@ @bar@");

    mapping.set("bar", "42");

    generator.append_pattern(" @foo@ @bar@");

    EXPECT_EQ(generator.generate(), "13 23 13 23");
}

TEST_MAIN(SourceGenerator)
