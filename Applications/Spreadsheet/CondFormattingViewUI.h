extern const char cond_fmt_view_ui_json[];
const char cond_fmt_view_ui_json[] = R"({
    "class": "GUI::Widget",
    "layout": {
        "class": "GUI::VerticalBoxLayout",
        "spacing": 2
    },
    "children": [
        {
            "class": "GUI::Widget",
            "layout": {
                "class": "GUI::HorizontalBoxLayout",
                "spacing": 10
            },
            "vertical_size_policy": "Fixed",
            "preferred_height": 25,
            "children": [
                {
                    "class": "GUI::Label",
                    "name": "if_label",
                    "horizontal_size_policy": "Fixed",
                    "vertical_size_policy": "Fixed",
                    "text": "if...",
                    "preferred_width": 40,
                    "preferred_height": 25
                },
                {
                    "class": "GUI::TextEditor",
                    "name": "formula_editor",
                    "horizontal_size_policy": "Fill",
                    "vertical_size_policy": "Fixed",
                    "tooltip": "Use 'value' to refer to the current cell's value",
                    "preferred_height": 25
                }
            ]
        },
        {
            "class": "GUI::Widget",
            "layout": {
                "class": "GUI::HorizontalBoxLayout",
                "spacing": 10
            },
            "vertical_size_policy": "Fixed",
            "preferred_height": 25,
            "children": [
                {
                    "class": "GUI::Label",
                    "name": "fg_color_label",
                    "horizontal_size_policy": "Fixed",
                    "vertical_size_policy": "Fixed",
                    "text": "Foreground...",
                    "preferred_width": 150,
                    "preferred_height": 25
                },
                {
                    "class": "GUI::ColorInput",
                    "name": "foreground_input",
                    "horizontal_size_policy": "Fill",
                    "vertical_size_policy": "Fixed",
                    "preferred_height": 25,
                    "preferred_width": 25
                }
            ]
        },
        {
            "class": "GUI::Widget",
            "layout": {
                "class": "GUI::HorizontalBoxLayout",
                "spacing": 10
            },
            "vertical_size_policy": "Fixed",
            "preferred_height": 25,
            "children": [
                {
                    "class": "GUI::Label",
                    "name": "bg_color_label",
                    "horizontal_size_policy": "Fixed",
                    "vertical_size_policy": "Fixed",
                    "text": "Background...",
                    "preferred_width": 150,
                    "preferred_height": 25
                },
                {
                    "class": "GUI::ColorInput",
                    "name": "background_input",
                    "horizontal_size_policy": "Fill",
                    "vertical_size_policy": "Fixed",
                    "preferred_height": 25,
                    "preferred_width": 25
                }
            ]
        }
    ]
}
)";
