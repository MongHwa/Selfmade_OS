#include "interrupt.hpp"
#include "queue.hpp"

usb::xhci::Controller* xhc;

//큐가 다루는 전용 데이터 타입
struct Message {
    enum Type {
        kInterruptXHCI,
    } type;
};

ArrayQueue<Message>* main_queue;

//이 함수는 인터럽트 핸들러라는 것을 알림
__attribute__((interrupt))
void IntHandlerXHCI(InterruptFrame* frame) {
    main_queue->Push(Message{Message::kInterruptXHCI});
    NotifyEndOfInterrupt();
}

//test - 메시지를 반복 처리하는 이벤트 루프 구조
while(true) {
    __asm__("cli"); //cpu의 인터럽트 플래그를 0으로. race condition 방지
    if(main_queue.Count() == 0) {
        __asm__("sti\n\thlt"); //sti + hlt
    }

    continue;
}

Message msg = main_queue.Front();
main_queue.Pop();
__asm__("sti"); //cpu의 인터럽트 플래그를 1로. 다음 인터럽트 받을 준비

switch(msg.type) {
    case Message::kInterruptXHCI:
        while(xhc.PrimaryEventRing()->HasFront()) {
            if(auto err = ProcessEvent(xhc)) {

            }
        }
        break;
    
    default:
        break;
}