//alignas()

extern "C" void KernelMainNewStack(const FrameBufferConfig& fream_buffer_config_ref,
                           const MemoryMap& memory_map_ref) {
    
    FrameBufferConfig frame_buffer_config{frame_buffer_config_ref};
    MemoryMap memory_map{memory_map_ref};
}