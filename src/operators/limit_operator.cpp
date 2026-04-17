#include "operators/limit_operator.h"

LimitOperator::LimitOperator(std::unique_ptr<IOperator> child, size_t limit, size_t offset)
    : child_(std::move(child)), limit_(limit), offset_(offset) {
}

std::optional<Batch> LimitOperator::Next() {
    auto batch_opt = child_->Next();
    if (!batch_opt.has_value()) {
        return std::nullopt;
    }
    Batch batch = std::move(*batch_opt);
    size_t batch_size = batch.RowsCnt();

    auto& cols = batch.GetAllColumns();
    for (auto& col : cols) {
        std::visit(
            [&]<typename T>(std::vector<T>& vec) {
                size_t size = std::min(limit_, batch_size - offset_);
                std::move(vec.begin() + offset_, vec.begin() + offset_ + size, vec.begin());
                vec.resize(size);
                vec.shrink_to_fit();
            },
            col.Value());
    }

    return batch;
}