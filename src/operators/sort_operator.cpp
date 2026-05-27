#include "operators/sort_operator.h"

#include <algorithm>
#include <functional>
#include <numeric>

SortOperator::SortOperator(std::unique_ptr<IOperator> child,
                           std::vector<std::pair<std::string, bool>>&& sort_columns, int limit,
                           int offset)
    : child_(std::move(child)),
      sort_columns_(std::move(sort_columns)),
      limit_(limit),
      offset_(offset) {
}

void ReorderBatch(Batch& batch, std::vector<size_t>& indices) {
    for (auto& column : batch.GetAllColumns()) {
        std::visit(
            [&indices]<typename T>(std::vector<T>& vec) {
                if (indices.empty()) {
                    vec.clear();
                } else {
                    std::vector<T> sorted;
                    sorted.reserve(indices.size());
                    for (size_t i : indices) {
                        sorted.push_back(std::move(vec[i]));
                    }
                    vec = std::move(sorted);
                }
            },
            column.Value());
    }
}

std::optional<Batch> SortOperator::Next() {
    std::optional<Batch> first_batch_opt = child_->Next();
    if (!first_batch_opt.has_value()) {
        return std::nullopt;
    }

    Batch batch = std::move(*first_batch_opt);

    auto build_comparator = [&](const Batch& batch) {
        std::vector<std::function<int(size_t, size_t)>> cmps;
        for (auto& [name, desc] : sort_columns_) {
            std::visit(
                [&cmps, desc](const auto& vec) {
                    cmps.push_back([&vec, desc](size_t a, size_t b) -> int {  // 1 = true (a < b)
                        if (vec[a] < vec[b]) {
                            return desc ? -1 : 1;
                        }
                        if (vec[a] > vec[b]) {
                            return desc ? 1 : -1;
                        }
                        return 0;
                    });
                },
                batch.GetColumn(name).Value());
        }

        return [cmps = std::move(cmps)](size_t a, size_t b) {
            for (const auto& comp : cmps) {
                int res = comp(a, b);
                if (res != 0) {
                    return res == 1;
                }
            }
            return false;
        };
    };

    while (auto b_opt = child_->Next()) {
        batch.Merge(std::move(*b_opt));

        if (limit_ == -1) {
            continue;
        }

        size_t k = static_cast<size_t>(limit_ + offset_);
        if (batch.RowsCnt() <= k) {
            continue;
        }

        std::vector<size_t> indices(batch.RowsCnt());
        std::iota(indices.begin(), indices.end(), 0);
        std::partial_sort(indices.begin(), indices.begin() + k, indices.end(),
                          build_comparator(batch));
        indices.resize(k);
        ReorderBatch(batch, indices);
    }

    if (batch.RowsCnt() <= static_cast<size_t>(offset_)) {
        batch.ClearValues();
        return batch;
    }

    std::vector<size_t> indices(batch.RowsCnt());
    std::iota(indices.begin(), indices.end(), 0);
    auto cmp = build_comparator(batch);

    if (limit_ == -1) {
        std::sort(indices.begin(), indices.end(), cmp);
    } else {
        size_t k = static_cast<size_t>(limit_ + offset_);
        std::partial_sort(indices.begin(), std::min(indices.begin() + k, indices.end()),
                          indices.end(), cmp);
        if (indices.size() > k) {
            indices.resize(k);
        }
        indices.erase(indices.begin(), indices.begin() + offset_);
    }

    ReorderBatch(batch, indices);

    return batch;
}
