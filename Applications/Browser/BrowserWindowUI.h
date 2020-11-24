extern const char browser_window_ui_json[];
const char browser_window_ui_json[] = R"({
    "name": "browser",
    "fill_with_background_color": true,

    "layout": {
        "class": "GUI::VerticalBoxLayout",
        "spacing": 2
    },

    "children": [
        {
            "class": "GUI::TabWidget",
            "name": "tab_widget",
            "container_padding": 0,
            "uniform_tabs": true,
            "text_alignment": "CenterLeft"
        }
    ]
}
)";
