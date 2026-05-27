#pragma once

#include <memory>

#include "base_operator.h"

class SortOperator : public IOperator {
public:
    SortOperator(std::unique_ptr<IOperator> child,
                 std::vector<std::pair<std::string, bool>>&& sort_columns, int limit = -1,
                 int offset = 0);

    std::optional<Batch> Next() override;

private:
    std::unique_ptr<IOperator> child_;
    std::vector<std::pair<std::string, bool>> sort_columns_;
    int limit_;
    int offset_;
};
