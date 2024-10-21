const int kMouseCursorWidth = 15;
const int kMouseCursorHeight = 24;
const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth+1] = {

};




//마우스 커서 렌더링
for(int dy = 0; dy < kMouseCursorHeight; dy++) {
    for(int dx = 0; dx < kMouseCursorWidth; dx++) {
        if(mouse_cursor_shape[dy][dx] == '@') {
            pixel_writer->Writer(200+dx, 100+dy, {0, 0, 0});
        } else if(mouse_cursor_shape[dy][dx] == '.') {
            pixel_writer->Writer(200+dx, 100+dy, {255, 255, 255});
        }
    }
}

//데스크톱 초기 화면 그리기
FillRectangle();