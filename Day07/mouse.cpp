#include "mouse.hpp"

MouseCursor::MouseCursor(PixelWriter* writer, PixelColor erase_color,
                         Vector2D<int> initial_position) 
    
    : pixel_writer_{writer},
      erase_color_{erase_color},
      position_{initial_position} {
    DrawMouseCursor(pixel_wrtier_, position_);
}

void MouseCursor::MoveRelative(Vector2D<int> displacement) {
    EraseMouseCursor(pixel_writer_, position_, erase_color_);
    position_ += displacement;
    DrawMouseCursor(pixel_writer_, position_);
}