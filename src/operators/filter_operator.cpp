#include "operators/filter_operator.h"

#include <algorithm>
#include <stdexcept>

#include "scheme/batch.h"

FilterOperator::FilterOperator(std::unique_ptr<IOperator> child,
                               std::unique_ptr<BoolExpression> expr)
    : child_(std::move(child)), expr_(std::move(expr)) {
}

Column ApplyMaskToColumn(const Column &column, const std::vector<bool> &mask) {
    return std::visit(
        [&mask, type = column.GetType()]<typename T>(const std::vector<T> &vec) -> Column {
            if (vec.size() != mask.size()) {
                throw std::runtime_error("Mask size does not match column size");
            }

            std::vector<T> data;
            for (size_t i = 0; i < vec.size(); ++i) {
                if (mask[i]) {
                    data.push_back(vec[i]);
                }
            }

            return Column(type, std::move(data));
        },
        column.Value());
}

std::optional<Batch> FilterOperator::Next() {
    while (true) {
        auto batch_opt = child_->Next();
        if (!batch_opt.has_value()) {
            return std::nullopt;
        }
        Batch batch = std::move(*batch_opt);

        std::vector<bool> mask = expr_->Evaluate(batch);
        if (std::find(mask.begin(), mask.end(), true) == mask.end()) {
            continue;
        }

        std::vector<Column> filtered;
        for (size_t i = 0; i < batch.ColumnsCnt(); ++i) {
            filtered.emplace_back(ApplyMaskToColumn(batch.GetColumn(i), mask));
        }

        return Batch(std::move(filtered), batch.GetScheme());
    }
}