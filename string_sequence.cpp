#include "string_sequence.hpp"
#include "str.hpp"

namespace NAC {
    void TBlobSequence::Concat(const std::shared_ptr<TBlob>& data) {
        Memorize(data);
        Sequence.emplace_back(TItem{data->Size(), data->Data()});
    }

    void TBlobSequence::Concat(const TBlob& data) {
        Sequence.emplace_back(TItem{data.Size(), data.Data()});
    }

    void TBlobSequence::Concat(TBlob&& data) {
        Sequence.emplace_back(TItem{data.Size(), data.Data()});

        if (data.Owning()) {
            MemorizeFree(data.Data());
        }

        data.Reset();
    }

    void TBlobSequence::Concat(const TBlobSequence& data) {
        MemorizeCopy(&data);

        for (const auto& node : data.Sequence) {
            Sequence.emplace_back(node);
        }
    }
}
