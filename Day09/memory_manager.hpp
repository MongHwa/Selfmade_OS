#include <stdint.h>

namespace {
    constexpr unsigned long long operator""_KiB (unsigned long long kib) {
        return kib * 1024;
    }

    constexpr unsigned long long operator""_MiB (unsigned long long mib) {
        return mib * 1024_KiB;
    }

    constexpr unsigned long long operator""_GiB (unsigned long long gib) {
        return gib * 1024_MiB;
    }
}

//프레임 하나의 크기
static const auto kBytesPerFrame{4_KiB};

class FrameID {
    public:
        explicit FrameID(size_t id) : id_{id} {}
        size_t ID() const { 
            return id_;
        }
        void* Frame() const {
            return reinterpret_cast<void*>(id_ * kBytesPerFrame);
        }
    
    private:
        size_t id_;
};

static const FrameID kNullFrame{std::numeric_limits<size_t>::max()};


//비트맵 방식으로 메모리 관리하기
class BitmapMemoryManager {
    public:
        static const auto kMaxPhysicalMemoryBytes{128_GiB};
        //kMaxPhysicalMemoryBytes까지 물리 메모리를 다루기 위해 필요한 프레임 수
        static const auto kFrameCount{kMaxPhysicalMemoryBytes / kBytesPerFrame};

        using MapLineType = unsigned long;
        //비트맵 배열 한 개의 요소 비트 수 == 프레임 수
        static const size_t kBitsPerMapLine{8 * sizeof(MapLineType)};

        //인스턴스 초기화
        BitmapMemoryManager();

        WithError<FrameID> Allocate(size_t num_frames);
        Error Free(FrameID start_frame, size_t num_frames);
        void MarkAllocated(FrameID start_frame, size_t num_frames);

        void SetMemoryRange(FrameID range_begin, FrameID range_end);
    
    private:
        std::array<MapLine, kFrameCount / kBitsPerMapLine> alloc_map_;
        FrameID range_begin_;
        FrameID range_end_;

        bool GetBit(FrameID frame) const;
        void SetBit(FrameID frame, bool allocated);
};

Error InitializeHeap(BitmapMemoryManager& memory_manager);