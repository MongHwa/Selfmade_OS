#pragma once

#include "graphics.hpp"

class Console {
    public:
        static const int kRows = 25, kColumns = 80;

        Console(PixelWriter& writer, 
            const PixelColor& fg_color, const PixelColor& bg_color);
        void PutString(const char* s);
        void SetWriter(PixelWriter* writer);
        void SetWindow(const std::shared_ptr<Window>& window);

    private:
        void Newline();

        PixelWriter& writer_;
        const PixelColor fg_color_, bg_color_;
        char buffer_[kRows][kColumns + 1];
        int cursor_row_, cursor_column_;
};

void Console::Refresh() {
    for(int row = 0; row < kRows; row++) {
        WriteString(*writer_, 0, 16*row, buffer_[row], fg_color_);
    }
}