extern const char cond_fmt_ui_json[];
const char cond_fmt_ui_json[] = R"({
    "name": "main",
    "fill_with_background_color": true,

    "layout": {
        "class": "GUI::VerticalBoxLayout",
        "spacing": 4
    },

    "children": [
        {
            "class": "Spreadsheet::ConditionsView",
            "name": "conditions_view"
        },
        {
            "class": "GUI::Widget",
            "vertical_size_policy": "Fixed",
            "horizontal_size_policy": "Fill",
            "preferred_width": 0,
            "preferred_height": 20,
            "layout": {
                "class": "GUI::HorizontalBoxLayout",
                "spacing": 10
            },
            "children": [
                {
                    "class": "GUI::Button",
                    "name": "add_button",
                    "text": "Add",
                    "horizontal_size_policy": "Fixed",
                    "vertical_size_policy": "Fixed",
                    "preferred_width": 100,
                    "preferred_height": 20
                },
                {
                    "class": "GUI::Button",
                    "name": "remove_button",
                    "text": "Remove",
                    "horizontal_size_policy": "Fixed",
                    "vertical_size_policy": "Fixed",
                    "preferred_width": 100,
                    "preferred_height": 20
                }
            ]
        }
    ]
}
)";
