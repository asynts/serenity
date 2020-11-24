extern const char tab_ui_json[];
const char tab_ui_json[] = R"({
    "layout": {
        "class": "GUI::VerticalBoxLayout"
    },

    "children": [
        {
            "class": "GUI::ToolBarContainer",
            "name": "toolbar_container",
            "children": [
                {
                    "class": "GUI::ToolBar",
                    "name": "toolbar"
                }
            ]
        },
        {
            "class": "GUI::Widget",
            "name": "webview_container",
            "layout": {
                "class": "GUI::VerticalBoxLayout"
            }
        },
        {
            "class": "GUI::StatusBar",
            "name": "statusbar"
        }
    ]
}
)";
