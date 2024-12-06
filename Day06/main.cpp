#include <cstdint>
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "pci.hpp"
#include "mouse.hpp"

int WritePixel(const FrameBufferConfig& config,
               int x, int y, const PixelColor& c) {
    
    const int pixel_position = config.pixels_per_scan_line*y + x;
    if(config.pixel_format == kPixelRGBResv8BitPerColor) {
        uint8_t* p = &config.frame_buffer[4 * pixel_position];
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    } else if(config.pixel_format == kPixelBGRResv8BitPerColor) {
        uint8_t* p = &config.frame_buffer[4 * pixel_position];
        p[0] = c.b;
        p[1] = c.g;
        p[2] = c.r;
    } else {
        return -1;
    }

    return 0;
}

//for pixel_writer pointer
//start of code
char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;
//end of code


//for displacement new
//start of code
void* operator new(size_t size, void* buf) {
    return buf;
}

void operator delete(void* obj) noexcept {
}
//end of code


//for printk() function
//start of code
int printk(const char* format, ...) {
    va_list ap;
    int result;
    char s[1024];

    va_start(ap, format);
    result = vsprintf(s, format, ap);
    va_end(ap);

    console->PutString(s);
    return result;
}
//end of code

//for mouse cursor 
//start of code
const int kMouseCursorWidth = 15;
const int kMouseCursorHeight = 24;
const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth+1] = {
    "@              ",
    "@@             ",
    "@.@            ",
    "@..@           ",
    "@...@          ",
    "@....@         ",
    "@.....@        ",
    "@......@       ",
    "@.......@      ",
    "@........@     ",
    "@.........@    ",
    "@..........@   ",
    "@...........@  ",
    "@............@ ",
    "@......@@@@@@@@",
    "@......@       ",
    "@....@@.@      ",
    "@...@ @.@      ",
    "@..@   @.@     ",
    "@.@    @.@     ",
    "@@      @.@    ",
    "@       @.@    ",
    "         @.@   ",
    "         @@@   ",
};
//end of code

//for MouseObserver function (Polling)
//start of code
char mouse_cursor_buf[sizeof(MouseCursor)];
MouseCursor* mouse_cursor;

void MouseObserver(int8_t displacement_x, int8_t displacement_y) {
    mouse_cursor->MoveRelative({displacement_x, displacement_y});
}
//end of code


extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {

    switch(frame_buffer_config.pixel_format) {
        case kPixelRGBResv8BitPerColor:
            pixel_writer = new(pixel_writer_buf)
                RGBResv8BitPerColorPixelWriter{frame_buffer_config};
            break;
        case kPixelBGRResv8BitPerColor:
            pixel_writer = new(pixel_writer_buf)
                BGRResv8BitPerColorPixelWriter{frame_buffer_config};
            break;
    }

    for(int x = 0; x < frame_buffer_config.horizontal_resolution; x++) {
        for(int y = 0; y < frame_buffer_config.vertical_resolution; y++) {
            pixel_writer->Write(x, y, {255, 255, 255});
        }
    }

    for(int x = 0; x < 200; x++) {
        for(int y = 0; y < 100; y++) {
            pixel_writer->Write(100+x, 100+y, {0, 255, 0});
        }
    }

    //마우스 커서 렌더링
    for(int dy = 0; dy < kMouseCursorHeight; dy++) {
        for(int dx = 0; dx < kMouseCursorWidth; dx++) {
            if(mouse_cursor_shape[dy][dx] == '@') {
                pixel_writer->Write(200+dx, 100+dy, {0, 0, 0});
            } else if(mouse_cursor_shape[dy][dx] == '.') {
                pixel_writer->Write(200+dx, 100+dy, {255, 255, 255});
            }
        }
    }

    //데스크톱 화면 그리기
    FillRectangle(*pixel_writer, {0, 0}, {kFrameWidth, kFrameHeight-50}, kDesktopBGColor);
    FillRectangle(*pixel_writer, {0, kFrameHeight-50}, {kFrameWidth, 50}, {1, 8, 7});
    FillRectangle(*pixel_writer, {0, kFrameHeight-50}, {kFrameWidth/5, 50}, {80, 80, 80});
    DrawRectangle(*pixel_writer, {10, kFrameHeight-40}, {30, 30}, {160, 160, 160});


    //pci 디바이스 열거
    //start of code
    auto err = pci::ScanAllBus();
    printk("ScanAllBus: %s\n", err.Name());

    for(int i = 0; i < pci::num_device; i++) {
        const auto& dev = pci::devices[i];
        auto vendor_id = pci::ReadVendorId(dev.bus, dev.device, dev.function);
        auto class_code = pci::ReadClassCode(dev.bus, dev.device, dev.function);
        printk("%d.%d.%d: vend %04x, class %08x, head %02x\n",
            dev.bus, dev.device, dev.function,
            vendor_id, class_code, dev.header_type);
    }
    //end of code
    

    //PCI 버스로부터 xHC 찾기
    pci::Device* xhc_dev = nullptr;
    for(int i = 0; i < pci::num_devices; i++) {
        if(pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x30u)) {
            xhc_dev = &pci::devices[i];

            //Intel 제품 우선
            if(0x8086 == pci::ReadVendorId(*xhc_dev)) {
                break;
            }
        }
    }

    if(xhc_dev) {
        Log(kInfo, "xHC has been found: %d.%d.%d\n",
            xhc_dev->bus, xhc_dev->device, xhc_dev->function);
    }

    //Read BAR0 register
    const WithError<uint64_t> xhc_bar = pci::ReadBar(*xhc_dev, 0);
    Log(kDebug, "ReadBar: %s\n", xhc_bar.error.Name());
    const uint64_t xhc_mmio_base = xhc_bar.value & ~static_cast<uint64_t>(0xf); //하위 4비트 제거. 이 값의 하위 4비트는 BAR의 플래그 값임.
    Log(kDebug, "xHC mmio_base = %08lx\n", xhc_mmio_base);

    //init xHC and run
    usb::xhci::Controller xhc{xhc_mmio_base}; //usb 드라이버 프로그램의 일부분. (제작해야함)

    if(0x8086 == pci::ReadVendorId(*xhc_dev)) { //Intel의 경우에만..
        SwitchEhci2Xhci(*xhc_dev); //초기 EHCI 제어모드를 XCHI 제어모드로 변경
    }

    auto err = xhc.Initallize();
    Log(kDebug, "xhc.Initialize: %s\n", err.Name());

    Log(kInfo, "xHC starting\n");
    xHC.Run();


    //USB포트 조사 후 연결된 포트의 설정 수행
    usb::HIDMouseDriver::default_observer = MouseObserver;

    for(int i = 1; i <= xhc.MaxPorts(); i++) {
        auto port = xhc.PortAt(i);
        Log(kDebug, "Port %d: IsConeected=%d\n", i, port.IsConnected());

        if(port.IsConnected()) {
            if(auto err = ConfigurePort(xhc, port)) {
                Log(kError, "failed to configure port: %s at %s:%d\n",
                    err.Name(), err.File(), err.Line());
                continue;
            }
        }
    }


    //xhc에 쌓인 이벤트 처리
    while(1) {
        if(auto err = ProcessEvent(xhc)) {
            Log(kError, "Error while ProcessEvent: %s at %s:%d\n",
                err.Name(), err.File(), err.Line());
        }
    }


    //MouseCursor 인스턴스 생성
    mouse_cursor = new(mouse_cursor_buf) MouseCursor {
        pixel_writer, kDesktopBGColor, {300, 200}
    };
    

    while(1) __asm__("hlt");
}


