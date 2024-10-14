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
        &map->descriptor_version
    );
}

/*
[structure of gBs->GetMemoryMap()]

EFI_STATUS GetMemoryMap(
    IN OUT UINTN *MemoryMapSize,
    IN OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
    OUT UINTN *MapKey,
    OUT UINTN *DescriptorSize,
    OUT UINT32 *DescriptorVersion
);

*/


EFI_STATUS SaveMemoryMap(struct MemoryMap* map, EFI_FILE_PROTOCOL* file) {
    //save as CSV

    CHAR8 buf[256];
    UINTN len;

    CHAR8* header = 
        "Index, Type, Type(name), PhysicalStart, NumberOfPages, Attribute\n";
    len = AsciiStrLen(header);
    file->Write(file, &len, header);

    Print(L"map->buffer = %08lx, map->map_size = %08lx\n",
        map->buffer, map->map_size);
    
    EFI_PHYSICAL_ADDRESS iter;
    int i;
    for(iter = (EFI_PHYSICAL_ADDRESS)map->buffer, i = 0;
        iter < (EFI_PHYSICAL_ADDRESS)map->buffer + map->map_size;
        iter += map->descriptor_size, i++) 
    {
        //정수를 포인터로 캐스트하면 그 정수를 주소로 하는 포인터로 변환됨.
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)iter;
        len = AsciiSPrint(
            buf, sizeof(buf),
            "%u, %x, %-ls, %08lx, %lx, %lx\n",
            i, dexc->Type, GetMemoryTypeUnicode(desc->Type),
            desc->PhysicalStart, desc->NumberOfPages,
            desc->Attribute & 0xffffflu
        );

        file->Write(file, &len, buf);
        /*Task : 지정한 문자열 전부 출력하지 못했을 때 남은 문자열 출력하는 프로그램 작성*/
    }

    return EFI_SUCCESS;
}


//For Main()
CHAR8 memmap_buf[4096*4];
struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
GetMemoryMap(&memmap);

EFI_FILE_PROTOCOL* root_dir;
OpenRootDir(image_handle, &root_dir); //open the file which is for writing

EFI_FILE_PROTOCOL* memmap_file;
root_dir->Open(
    root_dir, &memmap_file, L"\\memmap",
    EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0
); //double pointer technique

SaveMemoryMap(&memmap, memmap_file);
memmap_file->Close(memmap_file);

