extern const char main_window_ui_json[];
const char main_window_ui_json[] = R"({
    "name": "main",

    "layout": {
        "class": "GUI::VerticalBoxLayout",
        "spacing": 2
    },

    "children": [
        {
            "class": "Writer::WriterWidget",
            "name": "writer",

            "layout": {
                "class": "GUI::VerticalBoxLayout",
                "spacing": 2
            },

            "children": [
                {
                    "class": "Web::InProcessWebView",
                    "name": "web_view"
                }
            ]
        }
    ]
}
)";
