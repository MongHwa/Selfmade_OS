#include <Uefi.h>
#include <Library/UefiLib.h>

EFI_STATUS GetMemoryMap(struct MemoryMap* map) {
    if(map->buffer == NULL) {
        return EFI_BUFFER_TOO_SMALL;
    }

    map->map_size = map->buffer_size;
    return gBS->GetMemoryMap(
        &map->map_size,
        (EFI_MEMORY_DESCRIPTOR*)map->buffer,
        &map->map_key,
        &map->descriptor_size,
        &map->descriptor_version);
}

/*Main is HERE*/
EFI_STATUS EFIAPI UefiMain(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE* system_table) {
    
    Print(L"Hello, Mikan World!\n");
    while(1);
    return EFI_SUCCESS;
}
/*End of Main*/

//Read Kernel File
EFI_FILE_PROTOCOL* kernel_file;
root_dir->Open(
    root_dir, &kernel_file, L"\\kernel.elf",
    EFI_FILE_MODE_READ, 0);

UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16)*12;
UINT8 file_info_buffer[file_info_size];
kernel_file->GetInfo(
    kernel_file, &gEfiFileInfoGuid,
    &file_info_size, file_info_buffer);

EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
UINTN kernel_file_size = file_info->FileSize;

EFI_PHYSICAL_ADDRESS kernel_base_addr = 0x100000;
gBS->AllocatePages(
    AllocateAddress, EfiLoaderData,
    (kernel_file_size + 0xfff) / 0x1000, &kernel_base_addr);
kernel_file->Read(kernel_file, &kernel_file_size, (VOID*)kernel_base_addr);
Print(L"Kernel: 0x%0lx (%lu bytes)\n", kernel_base_addr, kernel_file_size);

//Stop BootService before starting kernel
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
        Print(L"Could not exit boot service: %r\n", status);
        while(1);
    }
}

//Start kernel
//start of code
UINT64 entry_addr = *(UINT64*)(kernel_base_addr + 24);

typedef void EntryPointType(const struct FrameBufferConfing*,
                            const struct MemoryMap*);
EntryPointType* entry_point = (EntryPointType*)entry_addr;
entry_point(&config, &memmap);
//end of code


//Writing Pixel in BootLoader
EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
OpenGOP(image_handle, &gop);

UINT8* frame_buffer = (UINT8*)gop->Mode->FrameBufferBase;
for(UINTN i = 0; i < gop->Mode->FrameBufferSize; i++) {
    frame_buffer[i] = 255;
}


//For improving Loader
//Start of code
EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
UINTN kernel_file_size = file_info->FileSize;

VOID* kernel_buffer;
status = gBS->AllocatePool(EFILoaderData, kernel_file_size, &kernel_buffer);
if(EFI_ERROR(status)) {
    Print(L"failed to allocate pool: %r\n", status);
    Halt();
}

status = kernel_file->Read(kernel_file, &kernel_file_size, kernel_buffer);
if(EFI_ERROR(status)) {
    Print(L"error: %r", status);
    Halt();
} //Read Kernel File using gBS->AllocatePool()

CalcLoadAddressRange(Elf64_Ehdr* ehdr, UINT64* first, UINT64* last) {
    Elf64_Phdr* phdr = (Elf64_Phdr*)((UINT64)ehdr + ehdr->e_phoff);
    *first = MAX_UINT64;
    *last = 0;

    for(Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
        if(phdr[i].p_type != PT_LOAD) {
            continue;
        }
        *first = MIN(*first, phdr[i].p_vaddr);
        *last = MAX(*last, phdr[i].p_vaddr + phdr[i].p_memsz);
    }
}

Elf64_Ehdr* kernel_ehdr = (Elf64_Ehdr*)kernel_buffer;
UINTN64 kernel_first_addr, kernel_last_addr;
CalcLoadAddressRange(kernel_ehdr, &kernel_first_addr, &kernel_last_addr);

UINTN num_pages = (kernel_last_addr - kernel_first_addr + 0xfff) / 0x1000;
status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, num_pages, &kernel_first_addr);

if(EFI_ERROR(status)) {
    Print(L"failed to allocate pages: %r\n", status);
    Halt();
} //Get Memory Area for Copying

void CopyLoadSegments(Elf64_Ehdr* ehdr) {
    Elf64_Phdr* phdr = (Elf64_Phdr*)((UINTN64)ehdr + ehdr->e_phoff);
    for(Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
        if(phdr[i].p_type != PT_LOAD) {
            continue;
        }

        UINT64 segm_in_file = (UINT64)ehdr + phdr[i].p_offset;
        CopyMem((VOID*)phdr[i].p_vaddr, (VOID*)segm_in_file, phdr[i].p_filesz);

        UINTN remain_bytes = phdr[i].p_memsz - phdr[i].p_filesz;
        SetMem((VOID*)(phdr[i].p_vaddr + phdr[i].p_filesz), remain_bytes, 0);
    }
}

CopyLoadSegments(kernel_ehdr);
Print(L"Kernel: 0x%0lx - 0x%0lx\n", kernel_first_addr, kernel_last_addr);

status = gBS->FreePool(kernel_buffer);
if(EFI_ERROR(status)) {
    Print(L"failed to free pool: %r\n", status);
    Halt();
} //Copy LOAD Segment