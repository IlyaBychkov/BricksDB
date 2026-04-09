#include "operators/aggregation_operator.h"
#include "scheme/batch.h"
#include "scheme/column.h"
#include "scheme/scheme.h"

CountState::CountState() = default;

void CountState::Update(const Column& column) {
    count_ += column.GetSize();
}

Column CountState::GetResult() {
    return Column(Type::int64, std::vector<int64_t>{count_});
}

SumState::SumState() = default;

void SumState::Update(const Column& column) {
    std::visit(
        [&]<typename T>(const std::vector<T>& vec) {
            if constexpr (std::is_arithmetic_v<T>) {
                for (const auto& val : vec) {
                    sum_ += static_cast<__int128_t>(val);
                }
            } else {
                throw std::runtime_error(
                    "SUM aggregation is not supported for non-numeric columns");
            }
        },
        column.Value());
}

Column SumState::GetResult() {
    return Column(Type::int64, std::vector<int64_t>{static_cast<int64_t>(sum_)});
}

AvgState::AvgState() = default;

void AvgState::Update(const Column& column) {
    std::visit(
        [&]<typename T>(const std::vector<T>& vec) {
            if constexpr (std::is_arithmetic_v<T>) {
                for (const auto& val : vec) {
                    sum_ += static_cast<__int128_t>(val);
                    count_++;
                }
            } else {
                throw std::runtime_error(
                    "AVG aggregation is not supported for non-numeric columns");
            }
        },
        column.Value());
}

Column AvgState::GetResult() {
    return Column(Type::int64,
                  std::vector<int64_t>{count_ > 0 ? static_cast<int64_t>(sum_ / count_) : 0});
}

AggregationOperator::AggregationOperator(
    std::unique_ptr<IOperator> child,
    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>>&& states)
    : child_(std::move(child)), states_(std::move(states)) {
}

std::optional<Batch> AggregationOperator::Next() {
    while (auto batch = child_->Next()) {
        for (auto& [agg_state, column_name] : states_) {
            agg_state->Update(batch->GetColumn(column_name));
        }
    }

    Scheme scheme;
    std::vector<Column> columns;
    for (auto& [agg_state, column_name] : states_) {
        Column col = agg_state->GetResult();
        scheme.AddElement(SchemeElement(column_name, col.GetType()));
        columns.push_back(col);
    }

    return Batch(std::move(columns), std::move(scheme));
}
