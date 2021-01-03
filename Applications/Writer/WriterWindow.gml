@GUI::Widget {
    name: "writer"
    fill_with_background_color: true

    layout: @GUI::VerticalBoxLayout {
    }

    @GUI::ToolBarContainer {
        @GUI::ToolBar {
            name: "toolbar"
        }
    }

    @Web::InProcessWebView {
        name: "webview"
    }

    @GUI::StatusBar {
        name: "statusbar"
    }
}
