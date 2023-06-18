#include "slag/message.h"

namespace slag {

    std::span<const uint64_t> Message::slots() const {
        return {
            slots_.data(),
            slots_.size(),
        };
    }

    std::span<const std::byte> Message::appendage() const {
        return {
            appendage_.data(),
            appendage_.size(),
        };
    }

    void Message::clear() {
        slots_.clear();
        appendage_.clear();
    }

    MessageWriter::MessageWriter(Message& message)
        : message_{message}
    {
    }

    void MessageWriter::write_flag(bool flag) {
        if (flag_slot_) {
            message_.slots_[flag_slot_->index] |= (1ull << flag_slot_->width);
            flag_slot_->width += 1;
            if (flag_slot_->width == MAX_MESSAGE_FLAG_SLOT_WIDTH) {
                flag_slot_.reset();
            }
        }
        else {
            flag_slot_ = MessageFlagSlot {
                .index = message_.slots_.size(),
                .width = 1,
            };

            write_slot(static_cast<uint64_t>(flag));
        }
    }

    void MessageWriter::write_slot(uint64_t slot) {
        message_.slots_.push_back(slot);
    }

    void MessageWriter::write_text(std::string_view text) {
        auto blob = std::as_bytes(
            std::span{text.data(), text.size()}
        );

        write_blob(blob);
    }

    void MessageWriter::write_blob(std::span<const std::byte> blob) {
        message_.appendage_.insert(
            message_.appendage_.end(),
            blob.begin(),
            blob.end()
        );
    }

    MessageReader::MessageReader()
        : message_{nullptr}
        , slot_index_{0}
        , appendage_offset_{0}
    {
    }

    MessageReader::MessageReader(const Message& message)
        : message_{&message}
        , slot_index_{0}
        , appendage_offset_{0}
    {
    }

    bool MessageReader::read_flag() {
        if (!message_) {
            return false;
        }

        if (!flag_slot_) {
            flag_slot_ = MessageFlagSlot {
                .index = slot_index_++,
                .width = 0,
            };
        }

        // extract the next flag from the slot
        bool flag = message_->slots_[flag_slot_->index] & (1ull << flag_slot_->width);
        flag_slot_->width += 1;

        // reset if the current flag slot has been exhausted
        if (flag_slot_->width == MAX_MESSAGE_FLAG_SLOT_WIDTH) {
            flag_slot_.reset();
        }

        return flag;
    }

    uint64_t MessageReader::read_slot() {
        if (!message_) {
            return 0;
        }

        return message_->slots_[slot_index_++];
    }

    std::string_view MessageReader::read_text(size_t size) {
        if (!message_) {
            return {};
        }

        const std::byte* data = &message_->appendage_[appendage_offset_];
        appendage_offset_ += size;

        return {
            reinterpret_cast<const char*>(data),
            size,
        };
    }

    std::span<const std::byte> MessageReader::read_blob(size_t size) {
        if (!message_) {
            return {};
        }

        const std::byte* data = &message_->appendage_[appendage_offset_];
        appendage_offset_ += size;

        return {
            data,
            size,
        };
    }

}
