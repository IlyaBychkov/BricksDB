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

    while (batch.RowsCnt() < limit_ + offset_) {
        const auto& b_opt = child_->Next();
        if (!b_opt.has_value()) {
            break;
        }

        const auto& new_cols = b_opt->GetAllColumns();
        for (size_t i = 0; i < batch.ColumnsCnt(); ++i) {
            std::visit(
                [&]<typename T>(std::vector<T>& vec) {
                    const auto& new_vec = std::get<std::vector<T>>(new_cols[i].Value());
                    vec.insert(std::end(vec), std::begin(new_vec), std::end(new_vec));
                },
                batch.GetColumn(i).Value());
        }
    }

    auto& cols = batch.GetAllColumns();
    for (auto& col : cols) {
        std::visit(
            [&]<typename T>(std::vector<T>& vec) {
                vec.erase(vec.begin(), vec.begin() + offset_);
                if (vec.size() > limit_) {
                    vec.resize(limit_);
                }
            },
            col.Value());
    }

    return batch;
}