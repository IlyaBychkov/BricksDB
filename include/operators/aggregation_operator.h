#pragma once

#include <limits>
#include <memory>

#include "operators/base_operator.h"

enum class AggregationType { COUNT, SUM, AVG, MIN, MAX };

struct AggregationState {
    int64_t count = 0;
    __int128_t sum = 0;
    int64_t min = std::numeric_limits<int64_t>::max();
    int64_t max = std::numeric_limits<int64_t>::min();

    void Update(const Column& col, AggregationType agg_type);

    Column GetResult(AggregationType agg_type, const std::string& column_name);
};

class AggregationOperator : public IOperator {
public:
    AggregationOperator(std::unique_ptr<IOperator> child,
                        const std::vector<std::pair<AggregationType, std::string>>& aggregations);

    std::optional<Batch> Next() override;

private:
    std::unique_ptr<IOperator> child_;
    std::vector<std::pair<AggregationType, std::string>> aggregations_;
    std::vector<AggregationState> states_;
};