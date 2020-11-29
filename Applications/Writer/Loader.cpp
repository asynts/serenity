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

#include <AK/JsonObject.h>
#include <Applications/Writer/Loader.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/Element.h>
#include <LibWeb/HTML/HTMLElement.h>
#include <LibWeb/HTML/Parser/HTMLDocumentParser.h>

namespace Writer {

static const char* writer_html_template = 1 + R"~~~(
<html>
    <head></head>
    <body></body>
</html>
)~~~";

static void decode_document(Web::DOM::Document& document, Web::DOM::Element& parent, const JsonObject& json)
{
    auto type = json.get("class").as_string();

    if (type == "paragraph") {
        auto element = document.create_element("p");

        for (const auto& child : json.get("children").as_array().values())
            decode_document(document, element, child.as_object());

        parent.append_child(element);
    } else if (type == "fragment") {
        auto element = document.create_element("span");
        element->set_text_content(json.get("content").as_string());

        if (json.get_or("bold", false).as_bool())
            element->class_names().append("bold");

        if (json.get_or("italic", false).as_bool())
            element->class_names().append("italic");

        parent.append_child(element);
    } else {
        ASSERT_NOT_REACHED();
    }
}

NonnullRefPtr<Web::DOM::Document> document_from_json(JsonValue json)
{
    Web::HTML::HTMLDocumentParser parser { writer_html_template, "utf-8" };
    parser.run("memory://writer");

    auto& document = parser.document();

    decode_document(document, *document.body(), json.as_object());

    return document;
}

static void encode_element(JsonObject& parent, const Web::DOM::Element& element)
{
    if (element.
}

JsonValue json_from_document(Web::DOM::Document& document)
{
    JsonObject object;

    encode_element(object, *document.body());

    return object;
}

}
