#include "operators/sort_operator.h"

#include <algorithm>
#include <numeric>

#include "operators/group_key.h"

SortOperator::SortOperator(std::unique_ptr<IOperator> child,
                           std::vector<std::pair<std::string, bool>>&& sort_columns)
    : child_(std::move(child)), sort_columns_(std::move(sort_columns)) {
}

std::optional<Batch> SortOperator::Next() {
    auto batch_opt = child_->Next();
    if (!batch_opt.has_value()) {
        return std::nullopt;
    }
    Batch batch = std::move(*batch_opt);

    std::vector<size_t> indices(batch.RowsCnt());
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        for (const auto& [col_name, desc] : sort_columns_) {
            auto val_a = std::visit([a](const auto& vect) -> ValueType { return vect[a]; },
                                    batch.GetColumn(col_name).Value());
            auto val_b = std::visit([b](const auto& vect) -> ValueType { return vect[b]; },
                                    batch.GetColumn(col_name).Value());

            if (val_a < val_b) {
                return !desc;
            }
            if (val_a > val_b) {
                return desc;
            }
        }
        return false;
    });
    for (auto& column : batch.GetAllColumns()) {
        std::visit(
            [&indices]<typename T>(std::vector<T>& vec) {
                std::vector<T> sorted_vec(vec.size());
                for (size_t i = 0; i < vec.size(); ++i) {
                    sorted_vec[i] = vec[indices[i]];
                }
                vec = std::move(sorted_vec);
            },
            column.Value());
    }
    return batch;
}