#include "operators/filter_operator.h"

#include <algorithm>
#include <stdexcept>

#include "scheme/batch.h"
#include "scheme/column.h"

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
    std::optional<Scheme> saved_scheme;
    while (auto batch_opt = child_->Next()) {
        if (!saved_scheme) {
            saved_scheme = batch_opt->GetScheme();
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

    if (saved_scheme) {
        std::vector<Column> empty_columns;
        for (size_t i = 0; i < saved_scheme->GetSize(); ++i) {
            empty_columns.emplace_back(saved_scheme->GetType(i));
        }

        return Batch(std::move(empty_columns), std::move(*saved_scheme));
    }
    return std::nullopt;
}
