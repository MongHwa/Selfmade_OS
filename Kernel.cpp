//C언어 형식으로 함수 정의
extern "C" void KernelMain() {
    while (1) __asm__("hlt");
}