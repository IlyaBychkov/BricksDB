#include <cstddef>
#include <unordered_set>

#include "operators/aggregation_operator.h"
#include "operators/group_key.h"
#include "scheme/batch.h"
#include "scheme/column.h"
#include "scheme/scheme.h"

CountState::CountState() = default;

void CountState::Update(const Column& column) {
    count_ += column.GetSize();
}

void CountState::UpdateOne(const ValueType& value) {
    count_ += 1;
}

Column CountState::GetResult() {
    return Column(Type::int64, std::vector<int64_t>{count_});
}

std::unique_ptr<AggregationState> CountState::Clone() {
    return std::make_unique<CountState>();
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

void SumState::UpdateOne(const ValueType& value) {
    std::visit(
        [&]<typename T>(const T& val) {
            if constexpr (std::is_arithmetic_v<T>) {
                sum_ += static_cast<__int128_t>(val);
            } else {
                throw std::runtime_error("SUM aggregation is not supported for non-numeric values");
            }
        },
        value);
}

Column SumState::GetResult() {
    return Column(Type::int64, std::vector<int64_t>{static_cast<int64_t>(sum_)});
}

std::unique_ptr<AggregationState> SumState::Clone() {
    return std::make_unique<SumState>();
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

void AvgState::UpdateOne(const ValueType& value) {
    std::visit(
        [&]<typename T>(const T& val) {
            if constexpr (std::is_arithmetic_v<T>) {
                sum_ += static_cast<__int128_t>(val);
                count_++;
            } else {
                throw std::runtime_error("AVG aggregation is not supported for non-numeric values");
            }
        },
        value);
}

Column AvgState::GetResult() {
    return Column(Type::int64,
                  std::vector<int64_t>{count_ > 0 ? static_cast<int64_t>(sum_ / count_) : 0});
}

std::unique_ptr<AggregationState> AvgState::Clone() {
    return std::make_unique<AvgState>();
}

AggregationOperator::AggregationOperator(
    std::unique_ptr<IOperator> child,
    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>>&& prototypes,
    std::vector<std::string>&& res_names, std::vector<std::string>&& group_columns_names)
    : child_(std::move(child)),
      prototypes_(std::move(prototypes)),
      res_names_(std::move(res_names)),
      group_columns_names_(std::move(group_columns_names)) {
}

std::optional<Batch> AggregationOperator::Next() {
    if (group_columns_names_.empty()) {
        while (auto batch = child_->Next()) {
            for (auto& [agg_state, column_name] : prototypes_) {
                agg_state->Update(batch->GetColumn(column_name));
            }
        }

        Scheme scheme;
        std::vector<Column> columns;
        for (size_t i = 0; i < prototypes_.size(); ++i) {
            const auto& [agg_state, column_name] = prototypes_[i];
            Column col = agg_state->GetResult();
            scheme.AddElement(SchemeElement(res_names_[i], col.GetType()));
            columns.push_back(col);
        }

        return Batch(std::move(columns), std::move(scheme));
    }

    std::unordered_set<GroupKey, GroupKeyHash> group_keys;
    std::vector<Type> keys_types;
    while (auto batch = child_->Next()) {
        std::vector<Column::ColumnValue> group_columns;
        for (const auto& name : group_columns_names_) {
            auto col = batch->GetColumn(name);
            group_columns.push_back(col.Value());
            if (keys_types.size() < group_columns.size()) {
                keys_types.push_back(col.GetType());
            }
        }

        std::vector<Column::ColumnValue> using_columns;
        for (const auto& [agg_state, column_name] : prototypes_) {
            using_columns.push_back(batch->GetColumn(column_name).Value());
        }

        size_t n = batch->RowsCnt();
        for (size_t i = 0; i < n; ++i) {
            GroupKey key;
            for (const auto& colv : group_columns) {
                key.push_back(
                    std::visit([i](const auto& vec) -> ValueType { return vec[i]; }, colv));
            }
            group_keys.insert(key);

            auto& agg_states = groups_[key];
            if (agg_states.empty()) {
                for (const auto& [prototype, column_name] : prototypes_) {
                    agg_states.push_back(prototype->Clone());
                }
            }

            for (size_t j = 0; j < prototypes_.size(); ++j) {
                agg_states[j]->UpdateOne(std::visit(
                    [i](const auto& vec) -> ValueType { return vec[i]; }, using_columns[j]));
            }
        }
    }

    Scheme scheme;
    std::vector<Column> columns;

    for (size_t i = 0; i < keys_types.size(); ++i) {
        columns.emplace_back(keys_types[i]);
        scheme.AddElement(SchemeElement(group_columns_names_[i], keys_types[i]));
    }
    for (size_t i = 0; i < prototypes_.size(); ++i) {
        const auto& [agg_state, column_name] = prototypes_[i];
        Type type = agg_state->GetResult().GetType();
        scheme.AddElement(SchemeElement(res_names_[i], type));
        columns.emplace_back(type);
    }

    for (const auto& key : group_keys) {
        for (size_t i = 0; i < key.size(); ++i) {
            std::visit([col = &columns[i]](const auto& val) { col->Push(val); }, key[i]);
        }

        for (size_t i = 0; i < prototypes_.size(); ++i) {
            auto& agg_state = groups_[key][i];
            auto val = std::visit([](const auto& vec) -> ValueType { return vec[0]; },
                                  agg_state->GetResult().Value());
            std::visit([col = &columns[key.size() + i]](const auto& val) { col->Push(val); }, val);
        }
    }

    return Batch(std::move(columns), std::move(scheme));
}
