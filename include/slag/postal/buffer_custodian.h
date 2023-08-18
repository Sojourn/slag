#pragma once



namespace slag::postal {

    class BufferCustodian {
    public:

    private:
        std::vector<BitSet>           references_;
        std::vector<BufferDescriptor> unused_buffers_;
    };

}
