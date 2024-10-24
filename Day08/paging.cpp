namespace {
    const uint64_t kPageSize4K = 4096;
    const uint64_t kPageSize2M = 512 * kPageSize4K;
    const uint64_t kPageSize1G = 512 * kPageSize2M;

    //alignas: 구조체 크기는 그대로. 구조체 선언시 주소를 파라미터의 배수로 설정.
    //__attribute((aligned))과의 차이점 ?
    alignas(kPageSize4K) std::array<uint64_t, 512> pml4_table;
    alignas(kPageSize4K) std::array<uint64_t, 512> pdp_table;
    alignas(kPageSize4K) std::array<std::array<uint64_t, 512>, kPageDirectoryCount> page_directory;
}

//64비트 모드에서의 페이징 설정 4계층 구조
//1. 페이지 맵 레벨(계층)4 테이블 (PML4 table)
//2. 페이지 디렉터리 포인터 테이블 (PDP table)
//3. 페이지 디렉터리
//4. 페이지 테이블
void SetupIdentityPageTable() {
    pml4_table[0] = reinterpret_cast<uint64_t>(&pdp_table[0]) | 0x003;
    for(int i_pdpt = 0; i_pdpt < page_director.size(); i_pdpt++) {
        pdp_table[i_pdpt] = reinterpret_cast<uint64_t>(&page_directory[i_pdpt]) | 0x003;
        
        for(int i_pd = 0; i_pd < 512; i_pd++) {
            page_directory[i_pdpt][i_pd] = i_pdpt*kPageSize1G + i_pd*kPageSize2M | 0x083;
        }
    }

    SetCR3(reinterpret_cast<uint64_t>(&pml4_table[0]));
}