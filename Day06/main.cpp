#include "pci.hpp"

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

/*if(xhc_dev) {
    Log(kInfo, "xHC has been found: %d.%d.%d\n",
        xhc_dev->bus, xhc_dev->device, xhc_dev->function);
}*/

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