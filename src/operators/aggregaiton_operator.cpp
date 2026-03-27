#include "operators/aggregation_operator.h"
#include "scheme/batch.h"
#include "scheme/column.h"
#include "scheme/scheme.h"

void AggregationState::Update(const Column& col, AggregationType agg_type) {
    count += col.GetSize();
    if (agg_type == AggregationType::COUNT) {
        return;
    }
    std::visit(
        [&]<typename T>(const std::vector<T>& vec) {
            if constexpr (std::is_arithmetic_v<T>) {
                switch (agg_type) {
                    case AggregationType::SUM:
                        for (const T& val : vec) {
                            sum += static_cast<__int128_t>(val);
                        }
                        break;

                    case AggregationType::AVG:
                        for (const T& val : vec) {
                            sum += static_cast<__int128_t>(val);
                        }
                        break;

                    case AggregationType::MIN:
                        for (const T& val : vec) {
                            min = std::min(min, static_cast<int64_t>(val));
                        }
                        break;

                    case AggregationType::MAX:
                        for (const T& val : vec) {
                            max = std::max(max, static_cast<int64_t>(val));
                        }
                        break;

                    default:
                        break;
                }
            } else {
                throw std::runtime_error(
                    "Numeric aggregation is not supported for non-numeric columns");
            }
        },
        col.Value());
}

Column AggregationState::GetResult(AggregationType agg_type, const std::string& column_name) {
    // TODO: add Type::double for AVG
    int64_t result_value = 0;
    switch (agg_type) {
        case AggregationType::COUNT:
            result_value = count;
            break;
        case AggregationType::SUM:
            result_value = static_cast<int64_t>(sum);
            break;
        case AggregationType::AVG:
            result_value = count > 0 ? static_cast<int64_t>(sum / count) : 0;
            break;
        case AggregationType::MIN:
            result_value = count > 0 ? static_cast<int64_t>(min) : 0;
            break;
        case AggregationType::MAX:
            result_value = count > 0 ? static_cast<int64_t>(max) : 0;
            break;
    }
    return Column(Type::int64, std::vector<int64_t>{result_value});
}

AggregationOperator::AggregationOperator(
    std::unique_ptr<IOperator> child,
    const std::vector<std::pair<AggregationType, std::string>>& aggregations)
    : child_(std::move(child)), aggregations_(aggregations) {
    states_.resize(aggregations.size());
}

std::optional<Batch> AggregationOperator::Next() {
    while (auto batch = child_->Next()) {
        for (size_t i = 0; i < aggregations_.size(); ++i) {
            auto [agg_type, column_name] = aggregations_[i];
            states_[i].Update(batch->GetColumn(column_name), agg_type);
        }
    }

    Scheme scheme;
    std::vector<Column> columns;

    for (size_t i = 0; i < aggregations_.size(); ++i) {
        auto [agg_type, column_name] = aggregations_[i];
        Column col = states_[i].GetResult(agg_type, column_name);
        scheme.AddElement(SchemeElement(column_name, col.GetType()));
        columns.push_back(col);
    }

    return Batch(std::move(columns), std::move(scheme));
}
