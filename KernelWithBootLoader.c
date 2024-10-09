EFI_FILE_PROTOCOL* kernel_file;
root_dir->Open(
    root_dir, &kernel_file, L"\\kernel.elf",
    EFI_FILE_MODE_READ, 0
);

/* 
[EFI_FILE_INFO structure]

typedef struct {
    UINTN64 Size, FileSize, PhysicalSize;
    EFI_TIME CreateTime, LastAccessTime, ModificationTime;
    UINT64 Attribute;
    CHAR16 Attribute;
    CHAR16 FileName[];
} EFI_FILE_INFO

FileName의 크기는 초기에 0으로 계산됨

*/

//파일 이름 저장을 위해 sizeof(CHAR16)*12 바이트 정도 더 크게 메모리 확보
UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
UINT8 file_info_buffer[file_info_size];
kernel_file->GetInfo(
    kernel_file, &gEfiFileInfoGuid,
    &file_info_size, file_info_buffer
);

EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
UINTN kernel_file_size = file_info->FileSize;

EFI_PHYSICAL_ADDRESS kernel_base_addr = 0x100000;
gBS->AllocatePages(
    AllocateAddress, EfiLoaderData,
    (kernel_file_size + 0xfff) / 0x1000, &kernel_base_addr
    //UEFI에서 1페이지의 크기는 0X1000바이트(4KiB)
    //따라서 버려지는 부분이 없도록 0xfff 더한 후 0x1000으로 나눔
);
kernel_file->Read(kernel_file, &kernel_file_size, (VOID*)kernel_base_addr);
Print(L"Kernel: 0x%01x (%lu bytes)\n", kernel_base_addr, kernel_file_size);



//부트 서비스 정지
EFI_STATUS status;
status = gBS->ExitBootServices(image_handle, memmap.map_key);
if(EFI_ERROR(status)) {
    status = GetMemoryMap(&memmap);
    if(EFI_ERROR(status)) {
        Print(L"failed to get memory map: %r\n", status);
        while(1);
    }

    status = gBS->ExitBootServices(image_handle, memmap.map_key);
    if(EFI_ERROR(status)) {
        //두 번이나 에러나면 이상현상
        Print(L"Could not exit boot service: %r\n", status);
        while(1); //임시처리
    }
}


//for Main()
//커널 가동
UINTN entry_addr = *(UINT64*)(kernel_base_addr + 24);

typedef void EntryPointType(UINT64, UINT64);
EntryPointType* entry_point = (EntryPointType*)entry_addr;
entry_point(gop->Mode->FrameBufferBase, gop->Mode->FrameBufferSize);

//((EntryPointType*)entry_addr)(); 과 같이 호출하는 것도 가능

