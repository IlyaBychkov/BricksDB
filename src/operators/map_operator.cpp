#include "operators/map_operator.h"

#include "scheme/column.h"

MapOperator::MapOperator(
    std::unique_ptr<IOperator> child,
    std::vector<std::pair<std::unique_ptr<IExpression>, std::string>>&& expressions)
    : child_(std::move(child)), expressions_(std::move(expressions)) {
}

std::optional<Batch> MapOperator::Next() {
    auto batch_opt = child_->Next();
    if (!batch_opt) {
        return std::nullopt;
    }
    Batch batch = std::move(*batch_opt);

    std::vector<std::pair<Column, std::string>> new_columns;
    for (auto& [expr, name] : expressions_) {
        Column col = expr->Evaluate(batch);
        batch.AddColumn(std::move(col), SchemeElement(name, col.GetType()));
    }
    return batch;
}