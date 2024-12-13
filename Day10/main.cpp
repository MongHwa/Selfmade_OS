#include <cstdint>
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "pci.hpp"
#include "mouse.hpp"
#include "interrupt.hpp"
#include "queue.hpp"
#include "memory_map.hpp"
#include "memory_manager.hpp"
#include "window.hpp"
#include "layer.hpp"
#include "frame_buffer.hpp"

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
//Day10 - Control the coordinate of mouse && Mouse Drag
//start of code
char mouse_cursor_buf[sizeof(MouseCursor)];
MouseCursor* mouse_cursor;
unsigned int mouse_layer_id;
Vector2D<int> screen_size;
Vector2D<int> mouse_position;

void MouseObserver(int8_t buttons, int8_t displacement_x, int8_t displacement_y) {
    static unsigned int mouse_drag_layer_id = 0;
    static uint8_t previous_buttons = 0;

    const auto oldpos = mouse_position;
    auto newpos = mouse_position + Vector2D<int>{displacement_x, displacement_y};
    newpos = ElementMin(newpos, scree_size + {Vector2D<int>{-1, -1}});
    mouse_position = ElementMin(newpos, {0, 0});

    const auto posdiff = mouse_position - oldpos;

    layer_manager->Move(mouse_layer_id, mouse_position);
    
    const bool previous_left_pressed = (previous_buttons & 0x01);
    const bool left_pressed = (buttons & 0x01);
    if(!previous_left_pressed && left_pressed) { //왼쪽 버튼 누른 순간
        auto layer = layer_manager->FindLayerByPosition(mouse_position, mouse_layer_id);
        if(layer) {
            mouse_drag_layer_id = layer->ID();
        }
    } else if(previous_left_pressed && left_pressed) { //왼쪽 버튼 누르는 동안
        if(mouse_drag_layer_id > 0) {
            layer_manager->MoveRelative(mouse_drag_layer_id, posdiff);
        }
    } else if(previous_left_presed && !left_pressed) { //왼쪽 버튼 떼는 순간
        mouse_drag_layer_id = 0;
    }

    previous_buttons = buttons;
}
//end of code

//for definition xHCI interrupt handler
//start of code
usb::xchi::Controller* xhc;

__attribute__((interrupt))
void IntHandlerXHCI(InterruptFrame* frame) {
    while(xhc->PrimaryEventRing()->HasFront()) {
        if(auto err == ProcessEvent(*xhc)) {
            Log(kError, "Error while ProcessEvent: %s at %s:%d\n",
                err.Name(), err.File(), err.Line());
        }
    }

    NotifyEndOfInterrupt();
}
//end of code

//for fast interrupt using queue
//start of code
struct Message {
    enum Type {
        kInterruptXHCI,
    } type;
};

ArrayQueue<Message>* main_queue;

__attribute__((interrupt))
void IntHandlerXHCI(InterruptFrame* frame) {
    main_queue->Push(Message{Message::kInterruptXHCI});
    NotifyEndOfInterrupt();
}
//end of code


/*Main is HERE*/
alignas(16) uint8_t kernel_main_stack[1024 * 1024];

extern "C" void KernelMainNewStack(const FrameBufferConfig& frame_buffer_config_ref,
                                   const MemoryMap& memory_map_ref) {
    
    FrameBufferConfig& frame_buffer_config{frame_buffer_config_ref};
    MemoryMap memory_map{memory_map_ref};

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

    //MouseCursor 인스턴스 생성
    mouse_cursor = new(mouse_cursor_buf) MouseCursor {
        pixel_writer, kDesktopBGColor, {300, 200}
    };
    

    //IDT를 CPU에 등록
    const uint16_t cs = GetCS();
    SetIDTEntry(idt[InterruptVector::kXHCI], 
                MakeIDTAttr(DescriptorType::kInterruptGate, 0),
                reinterpret_cast<uint64_t>(IntHandlerXHCI), cs);
    LoadIDT(sizeof(idt)-1, reinterpret_cast<uintptr_t>(&idt[0]));

    //MSI 인터럽트 활성화
    const uint8_t bsp_local_apic_id = 
        *reinterpret_cast<const uint32_t*>(0xfee00020) >> 24;
    pci::ConfigureMSIFixedDestination(*xhc_dev,
                                      bsp_local_apic_id,
                                      pci::MSITriggerMode::kLevel,
                                      pci::MSIDeliveryMode::kFixed,
                                      InterruptVector::kXHCI, 0);

    //인터럽트 고속화 (메시지를 반복 처리하는 이벤트 루프 구조)
    while(true) {
        __asm__("cli");
        if(main_queue.Count() == 0) {
            __asm__("sti/n/thlt");
            continue;
        }

        Message msg = main_queue.Front();
        main_queue.Pop();
        __asm__("sti");

        switch(msg.type) {
            case Message::kInterruptXHCI:
                while(xhc.PrimaryEventRing()->HasFront()) {
                    if(auto err = ProcessEvent(xhc)) {
                        Log(kError, "Error while ProcessEvent: %s at %s:%d\n",
                            err.Name(), err.File(), err.Line());
                    }
                }
                break;
            
            default:
                Log(kError, "Unknown message type: %d\n", msg_type);
        }
    }


    //메모리 맵 표시 Test
    printk("memory_map: %p\n", &memory_map);
    for(uintptr_t iter = reinterpret_cast<uintptr_t>(memory_map_buffer);
        iter += memory_map.descriptor_size) {
        
        auto desc = reinterpret_cast<MemoryDescriptor*>(iter);
        for(int i = 0; i < available_memory_types.size(); i++) {
            if(desc->type == availabe_memory_types[i]) {
                printk("type = %u, phys = %08lx - %08lx, pages = %lu, attr = %08lx\n",
                    desc->type,
                    desc->physical_start,
                    dexc->physical_start + desc->number_of_pages*4096 - 1,
                    desc->number_of_pages,
                    desc->attribute);
            }
        }
    }

    //세그먼테이션 설정
    SetupSegments();                    //1. GDT 재구축

    const uint16_t kernel_cs = (1 << 3);//2. GDT를 CPU에 반영
    const uint16_t kernel_ss = (2 << 3);
    SetDSAll(0);
    SetCSSS(kernel_cs, kernel_ss);

    SetupIdentityPageTable();           //3. 페이징 설정


    //Day09 start
    memory_manager->SetMemoryRange(FrameID{1}, FrameID{available_end / kBytesPerFrame});

    if(auto err = InitializeHeap(*memory_manager)) {
        Log(kError, "failed to allocate pages: %s at %s:%d\n",
            err.Name(), err.File(), err.Line());
        exit(1);
    }

    //레이어 2개 생성(배경 데스크톱, 마우스)
    const int kFrameWidth = frame_buffer_config.horizontal_resolution;
    const int kFrameHeight = frame_buffer_config.vertical_resolution;

    auto bgwindow = std::make_shared<Window>(kFrameWidth, kFrameHeight);
    auto bgwriter = bgwindow->Writer();

    DrawDesktop(*bgwriter);
    console->SetWriter(bgwriter);

    auto mouse_window = std::make_shared<Window>(
        kMouseCursorWidth, kMouseCursorHeight);
    mouse_window->SetTransparentColor(kMouseTransparentColor);
    DrawMouseCursor(mouse_window->Writer(), {0, 0});

    layer_manager = new LayerManager;
    layer_manager->SetWriter(pixel_writer);

    auto bglayer_id = layer_manager->NewLayer()
        .SetWindow(bgwindow)
        .Move({0, 0});
        .ID();
    mouse_layer_id = layer_manager->NewLayer()
        .SetWindow(mouse_window)
        .Move({200, 200});
        .ID();

    layer_manager->UpDown(bglayer_id, 0);
    layer_manager->UpDown(mouse_layer_id, 1);
    layer_manager->Draw();

    //콘솔의 SetWriter() 호출
    DrawDesktop(*pixel_wrtier);

    console = new(console_buf) Console{
        kDestopFGColor, kDesktopBGColor
    };
    console->SetWriter(pixel_writer);
    printk("Welcome to MikanOS!\n");
    SetLogLevel(kWarn);

    //실제 프레임 버퍼를 나타내는 FrameBuffer 인스턴스 생성
    FrameBuffer screen;
    if(auto err = screen.Initialize(frame_buffer_config)) {
        Log(kError, "failed to initialize frame buffer: %s at %s:%d\n",
            err.Name(), err.File(), err.Line());
    }

    layer_manager = new LayerManager;
    layer_manager->SetWriter(&screen);

    //콘솔의 렌더링 타깃 윈도우 설정
    DrawDesktop(*bgwriter);
    console->SetWindow(bgwindow);


    //Day10 start
    screen_size.x = frame_buffer_config.horizontal_resolution;
    screen_size.y = frame_buffer_config.vertical_resolution;

    //테스트용 윈도우 생성 및 렌더링 & 레이어 생성 (카운터 역할)
    auto main_window = std::make_shared<Window>(
        160, 52 frame_buffer_config.pixel_format);
    DrawWindow(*main_window->Writer(), "Hello Window");
    
    auto main_window_layer_id = layer_manager->NewLayer()
        .SetWindow(main_window)
        .Move({300, 100})
        .ID();
    
    layer_manager->UpDown(bglayer_id, 0);
    layer_manager->UpDown(console->LayerID(), 1);
    layer_manager->UpDown(main_window_layer_id, 2);
    layer_manager->UpDown(mouse_layer_id, 3);
    layer_manager->Draw({{0, 0}, screen_size});

    //콘솔용 윈도우 및 레이어 생성
    auto console_window = std::make_shared<Window>(
        Console::kColumns * 8, Console::kRows * 16, frame_buffer_config.pixel_format);
    console->SetWindow(console_window);

    console->SetLayerID(layer_manager->NewLayer()
        .SetWindow(console_window)
        .Move({0, 0})
        .ID());

    //카운터 디테일 구현
    char str[128];
    unsigned int count = 0;

    while(1) {
        count++;
        sprintf(str, "%010u", count);
        FillRetangle(*main_window->Writer(), {24, 28}, {8 * 10, 16}, {0xc6, 0xc6, 0xc6});
        WriteString(*main_window->Writer(), {24, 28}, str, {0, 0, 0});
        layer_manager->Draw(main_window_layer_id);

        __asm__("cli");
        if(main_queue.Count() == 0) {
            __asm__("sti");
            continue;
        }
    }

    //while(1) __asm__("hlt");
}