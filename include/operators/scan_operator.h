#pragma once

#include <fstream>
#include <optional>
#include <set>

#include "operators/base_operator.h"
#include "transform/metadata.h"

class ScanOperator : public IOperator {
public:
    ScanOperator(const std::string& filename, const std::set<std::string>& columns);
    ~ScanOperator();

    std::optional<Batch> Next() override;

private:
    std::ifstream fin_;
    std::set<std::string> columns_;
    Metadata metadata_;
    size_t batch_num_ = 0;
};
