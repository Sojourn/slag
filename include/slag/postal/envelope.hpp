#include <cassert>

namespace slag {

    inline const PostCode& Envelope::to() const {
        return to_;
    }

    inline const PostCode& Envelope::from() const {
        return from_;
    }

    inline const PostageStamp& Envelope::stamp() const {
        return stamp_;
    }

    inline const BufferHandle& Envelope::content() const {
        return content_;
    }

    inline void Envelope::set_to(PostCode post_code) {
        to_ = post_code;
    }

    inline void Envelope::set_from(PostCode post_code) {
        from_ = post_code;
    }

    inline void Envelope::set_stamp(PostageStamp stamp) {
        stamp_ = stamp;
    }

    inline void Envelope::set_content(BufferHandle content) {
        content_ = std::move(content);
    }

}
