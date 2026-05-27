#include "operators/projection_operator.h"

#include <cstddef>

#include "scheme/scheme.h"

ProjectionOperator::ProjectionOperator(std::unique_ptr<IOperator> child,
                                       std::vector<std::string>&& projections)
    : child_(std::move(child)), projections_(std::move(projections)) {
}

std::optional<Batch> ProjectionOperator::Next() {
    auto batch_opt = child_->Next();
    if (!batch_opt.has_value()) {
        return std::nullopt;
    }

    const auto& batch = *batch_opt;
    size_t n = batch.ColumnsCnt();

    std::vector<SchemeElement> scheme_elements;
    std::vector<Column> res_cols;
    for (const auto& name : projections_) {
        bool found = false;
        for (size_t i = 0; i < n; ++i) {
            if (batch.GetColumnName(i) == name) {
                scheme_elements.push_back(SchemeElement(name, batch.GetColumnType(i)));
                res_cols.push_back(batch.GetColumn(i));
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::runtime_error("Projection Operator: Column not found: " + name);
        }
    }

    return Batch(std::move(res_cols), Scheme(std::move(scheme_elements)));
}
