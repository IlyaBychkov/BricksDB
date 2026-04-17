#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <functional>

#include "expressions/cmp_expressions.h"
#include "operators/aggregation_operator.h"
#include "operators/filter_operator.h"
#include "operators/scan_operator.h"
#include "scheme/batch.h"
#include "scheme/scheme.h"

class ClickBenchTest : public ::testing::Test {
protected:
    void SetUp() override {
        repo_root = "/home/ilya-bychkov/VsCodeProjects/BricksDB";
        hits_file = repo_root / "hits_files" / "hits_sample.br";
        out_dir = repo_root / "tests" / "operators" / "clickbench_results";
    }

    void WriteResult(const Batch& batch, const std::string& filename,
                     const std::string& scheme_filename) {
        auto wres = WriteBatchToCSV(batch, filename);
        ASSERT_TRUE(wres.has_value()) << "Failed to write batch: " << wres.error();

        auto sres = WriteSchemeToCSV(batch.GetScheme(), scheme_filename);
        ASSERT_TRUE(sres.has_value()) << "Failed to write scheme: " << sres.error();
    }

    void ExecuteAndVerify(std::unique_ptr<IOperator> root_op, int query_id) {
        auto res_batch = root_op->Next();
        ASSERT_TRUE(res_batch.has_value()) << "Query " << query_id << " returned no batch";

        std::string ans_path = (out_dir / (std::to_string(query_id) + "ans.csv")).string();
        std::string scheme_path = (out_dir / (std::to_string(query_id) + "scheme.csv")).string();

        WriteResult(*res_batch, ans_path, scheme_path);
    }

    std::filesystem::path repo_root;
    std::filesystem::path hits_file;
    std::filesystem::path out_dir;
};

TEST_F(ClickBenchTest, Query0) {
    auto scan_ptr =
        std::make_unique<ScanOperator>(hits_file.string(), std::set<std::string>{"WatchID"});

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountState>(), "WatchID");
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states));

    ExecuteAndVerify(std::move(agg_ptr), 0);
}

TEST_F(ClickBenchTest, Query1) {
    std::set<std::string> cols = {"AdvEngineID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr_ptr = std::make_unique<CompareExpression<std::not_equal_to<int16_t>, int16_t>>(
        "AdvEngineID", static_cast<int16_t>(0));
    auto filter_ptr = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr_ptr));

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountState>(), "AdvEngineID");
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(filter_ptr), std::move(states));

    ExecuteAndVerify(std::move(agg_ptr), 1);
}

TEST_F(ClickBenchTest, Query2) {
    std::set<std::string> cols = {"AdvEngineID", "ResolutionWidth"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<SumState>(), "AdvEngineID");
    states.emplace_back(std::make_unique<CountState>(), "AdvEngineID");
    states.emplace_back(std::make_unique<AvgState>(), "ResolutionWidth");
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states));

    ExecuteAndVerify(std::move(agg_ptr), 2);
}

TEST_F(ClickBenchTest, Query3) {
    std::set<std::string> cols = {"UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<AvgState>(), "UserID");
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states));

    ExecuteAndVerify(std::move(agg_ptr), 3);
}

TEST_F(ClickBenchTest, Query4) {
    std::set<std::string> cols = {"UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountDistinctState<int64_t>>(Type::int64), "UserID");
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states));

    ExecuteAndVerify(std::move(agg_ptr), 4);
}

TEST_F(ClickBenchTest, Query5) {
    std::set<std::string> cols = {"SearchPhrase"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountDistinctState<std::string>>(Type::string),
                        "SearchPhrase");
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states));

    ExecuteAndVerify(std::move(agg_ptr), 5);
}

TEST_F(ClickBenchTest, Query6) {
    std::set<std::string> cols = {"EventDate"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<MinState<int32_t>>(Type::date), "EventDate");
    states.emplace_back(std::make_unique<MaxState<int32_t>>(Type::date), "EventDate");
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states));

    ExecuteAndVerify(std::move(agg_ptr), 6);
}

TEST_F(ClickBenchTest, Query7) {
    std::set<std::string> cols = {"AdvEngineID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr_ptr = std::make_unique<CompareExpression<std::not_equal_to<int16_t>, int16_t>>(
        "AdvEngineID", static_cast<int16_t>(0));
    auto filter_ptr = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr_ptr));

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountState>(), "AdvEngineID");
    std::vector<std::string> group_columns_names = {"AdvEngineID"};
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(filter_ptr), std::move(states),
                                                         std::move(group_columns_names));

    ExecuteAndVerify(std::move(agg_ptr), 7);
}