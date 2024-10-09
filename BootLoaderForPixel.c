//For Main()
EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
OpenGop(image_handle, &gop); //GOP(Graphics Output Protocol) 취득
Print(L"Resolution: %ux%u, Pixel Format: %s, %u pixels/line\n",
    gop->Mode->Info->HorizontalResolution,
    gop->Mode->Info->VerticalResolution,
    GetPixelFormatUnicode(gop->Mode->Info->PixelFormat),
    gop->Mode->Info->PixelsPerScanLine
);

Print(L"Frame buffer: 0x%01x - 0x%01x, Size: %lu bytes\n",
    gop->Mode->FrameBufferBase,
    gop->Mode->FrameBufferBase + gop->Mode->FrameBufferSize,
    gop->Mode->FrameBufferSize
);

UINT8* frame_buffer = (UINT8*)gop->Mode->FrameBufferBase;
for(UINTN i = 0; i < gop->Mode->FrameBufferSize; i++) {
    frame_buffer[i] = 255;
    //255는 흰색. 어떤 데이터 형식에서도 마찬가지.
}