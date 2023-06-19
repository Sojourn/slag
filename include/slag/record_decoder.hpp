#include <stdexcept>

namespace slag {

    template<typename Handler>
    inline void decode(Handler&& handler, MessageReader& reader) {
        switch (static_cast<RecordType>(reader.read_slot())) {
#define X(SLAG_RECORD_TYPE)                                  \
            case RecordType::SLAG_RECORD_TYPE: {             \
                Record<RecordType::SLAG_RECORD_TYPE> record; \
                decode(record, reader);                      \
                handler(record);                             \
                return;                                      \
            }                                                \

            SLAG_RECORD_TYPES(X)
#undef X
        }

        throw std::runtime_error("Invalid record type");
    }

}
