extern const char inspector_window_ui_json[];
const char inspector_window_ui_json[] = R"({
    "name": "inspector",

    "layout": {
        "class": "GUI::VerticalBoxLayout"
    },

    "children": [
        {
            "class": "GUI::TreeView",
            "name": "dom_view"
        }
    ]
}
)";
