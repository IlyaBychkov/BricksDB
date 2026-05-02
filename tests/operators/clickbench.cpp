#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <functional>

#include "expressions/cmp_expressions.h"
#include "operators/aggregation_operator.h"
#include "operators/filter_operator.h"
#include "operators/limit_operator.h"
#include "operators/scan_operator.h"
#include "operators/sort_operator.h"
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
        std::string ans_path = (out_dir / (std::to_string(query_id) + "ans.csv")).string();
        std::string scheme_path = (out_dir / (std::to_string(query_id) + "scheme.csv")).string();

        auto res_batch = root_op->Next();

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
    std::vector<std::string> res_names = {"COUNT(*)"};
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states),
                                                         std::move(res_names));

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
    std::vector<std::string> res_names = {"COUNT(*)"};
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(filter_ptr), std::move(states),
                                                         std::move(res_names));

    ExecuteAndVerify(std::move(agg_ptr), 1);
}

TEST_F(ClickBenchTest, Query2) {
    std::set<std::string> cols = {"AdvEngineID", "ResolutionWidth"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<SumState>(), "AdvEngineID");
    states.emplace_back(std::make_unique<CountState>(), "AdvEngineID");
    states.emplace_back(std::make_unique<AvgState>(), "ResolutionWidth");
    std::vector<std::string> res_names = {"SUM(AdvEngineID)", "COUNT(*)", "AVG(ResolutionWidth)"};
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states),
                                                         std::move(res_names));

    ExecuteAndVerify(std::move(agg_ptr), 2);
}

TEST_F(ClickBenchTest, Query3) {
    std::set<std::string> cols = {"UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<AvgState>(), "UserID");
    std::vector<std::string> res_names = {"AVG(UserID)"};
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states),
                                                         std::move(res_names));

    ExecuteAndVerify(std::move(agg_ptr), 3);
}

TEST_F(ClickBenchTest, Query4) {
    std::set<std::string> cols = {"UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountDistinctState<int64_t>>(Type::int64), "UserID");
    std::vector<std::string> res_names = {"COUNT(DISTINCT UserID)"};
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states),
                                                         std::move(res_names));

    ExecuteAndVerify(std::move(agg_ptr), 4);
}

TEST_F(ClickBenchTest, Query5) {
    std::set<std::string> cols = {"SearchPhrase"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountDistinctState<std::string>>(Type::string),
                        "SearchPhrase");
    std::vector<std::string> res_names = {"COUNT(DISTINCT SearchPhrase)"};
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states),
                                                         std::move(res_names));

    ExecuteAndVerify(std::move(agg_ptr), 5);
}

TEST_F(ClickBenchTest, Query6) {
    std::set<std::string> cols = {"EventDate"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<MinState<int32_t>>(Type::date), "EventDate");
    states.emplace_back(std::make_unique<MaxState<int32_t>>(Type::date), "EventDate");
    std::vector<std::string> res_names = {"MIN(EventDate)", "MAX(EventDate)"};
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states),
                                                         std::move(res_names));

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
    std::vector<std::string> res_names = {"COUNT(*)"};
    auto agg_ptr =
        std::make_unique<AggregationOperator>(std::move(filter_ptr), std::move(states),
                                              std::move(res_names), std::move(group_columns_names));

    auto sort_columns = std::vector<std::pair<std::string, bool>>{{"COUNT(*)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_columns));

    ExecuteAndVerify(std::move(sort_ptr), 7);
}

TEST_F(ClickBenchTest, Query8) {
    std::set<std::string> cols = {"RegionID", "UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountDistinctState<int64_t>>(Type::int64), "UserID");
    std::vector<std::string> group_columns_names = {"RegionID"};
    std::vector<std::string> res_names = {"COUNT(DISTINCT UserID)"};
    auto agg_ptr =
        std::make_unique<AggregationOperator>(std::move(scan_ptr), std::move(states),
                                              std::move(res_names), std::move(group_columns_names));

    auto sort_columns = std::vector<std::pair<std::string, bool>>{{"COUNT(DISTINCT UserID)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_columns));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 8);
}

TEST_F(ClickBenchTest, Query9) {
    std::set<std::string> cols = {"RegionID", "AdvEngineID", "ResolutionWidth", "UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<SumState>(), "AdvEngineID");
    states.emplace_back(std::make_unique<CountState>(), "RegionID");
    states.emplace_back(std::make_unique<AvgState>(), "ResolutionWidth");
    states.emplace_back(std::make_unique<CountDistinctState<int64_t>>(Type::int64), "UserID");
    std::vector<std::string> res_names = {"SUM(AdvEngineID)", "COUNT(*)", "AVG(ResolutionWidth)",
                                          "COUNT(DISTINCT UserID)"};
    std::vector<std::string> group_cols = {"RegionID"};
    auto agg_ptr = std::make_unique<AggregationOperator>(
        std::move(scan_ptr), std::move(states), std::move(res_names), std::move(group_cols));

    auto sort_cols = std::vector<std::pair<std::string, bool>>{{"COUNT(*)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_cols));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 9);
}

TEST_F(ClickBenchTest, Query10) {
    std::set<std::string> cols = {"MobilePhoneModel", "UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr = std::make_unique<CompareExpression<std::not_equal_to<std::string>, std::string>>(
        "MobilePhoneModel", "");
    auto filter_ptr = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr));

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountDistinctState<int64_t>>(Type::int64), "UserID");
    std::vector<std::string> res_names = {"COUNT(DISTINCT UserID)"};
    std::vector<std::string> group_cols = {"MobilePhoneModel"};
    auto agg_ptr = std::make_unique<AggregationOperator>(
        std::move(filter_ptr), std::move(states), std::move(res_names), std::move(group_cols));

    auto sort_cols = std::vector<std::pair<std::string, bool>>{{"COUNT(DISTINCT UserID)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_cols));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 10);
}

TEST_F(ClickBenchTest, Query11) {
    std::set<std::string> cols = {"MobilePhone", "MobilePhoneModel", "UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr = std::make_unique<CompareExpression<std::not_equal_to<std::string>, std::string>>(
        "MobilePhoneModel", "");
    auto filter_ptr = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr));

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountDistinctState<int64_t>>(Type::int64), "UserID");
    std::vector<std::string> res_names = {"COUNT(DISTINCT UserID)"};
    std::vector<std::string> group_cols = {"MobilePhone", "MobilePhoneModel"};
    auto agg_ptr = std::make_unique<AggregationOperator>(
        std::move(filter_ptr), std::move(states), std::move(res_names), std::move(group_cols));

    auto sort_cols = std::vector<std::pair<std::string, bool>>{{"COUNT(DISTINCT UserID)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_cols));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 11);
}

TEST_F(ClickBenchTest, Query12) {
    std::set<std::string> cols = {"SearchPhrase"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr = std::make_unique<CompareExpression<std::not_equal_to<std::string>, std::string>>(
        "SearchPhrase", "");
    auto filter_ptr = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr));

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountState>(), "SearchPhrase");
    std::vector<std::string> res_names = {"COUNT(*)"};
    std::vector<std::string> group_cols = {"SearchPhrase"};
    auto agg_ptr = std::make_unique<AggregationOperator>(
        std::move(filter_ptr), std::move(states), std::move(res_names), std::move(group_cols));

    auto sort_cols = std::vector<std::pair<std::string, bool>>{{"COUNT(*)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_cols));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 12);
}

TEST_F(ClickBenchTest, Query13) {
    std::set<std::string> cols = {"SearchPhrase", "UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr = std::make_unique<CompareExpression<std::not_equal_to<std::string>, std::string>>(
        "SearchPhrase", "");
    auto filter_ptr = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr));

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountDistinctState<int64_t>>(Type::int64), "UserID");
    std::vector<std::string> res_names = {"COUNT(DISTINCT UserID)"};
    std::vector<std::string> group_cols = {"SearchPhrase"};
    auto agg_ptr = std::make_unique<AggregationOperator>(
        std::move(filter_ptr), std::move(states), std::move(res_names), std::move(group_cols));

    auto sort_cols = std::vector<std::pair<std::string, bool>>{{"COUNT(DISTINCT UserID)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_cols));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 13);
}

TEST_F(ClickBenchTest, Query14) {
    std::set<std::string> cols = {"SearchEngineID", "SearchPhrase"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr = std::make_unique<CompareExpression<std::not_equal_to<std::string>, std::string>>(
        "SearchPhrase", "");
    auto filter_ptr = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr));

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountState>(), "SearchPhrase");
    std::vector<std::string> res_names = {"COUNT(*)"};
    std::vector<std::string> group_cols = {"SearchEngineID", "SearchPhrase"};
    auto agg_ptr = std::make_unique<AggregationOperator>(
        std::move(filter_ptr), std::move(states), std::move(res_names), std::move(group_cols));

    auto sort_cols = std::vector<std::pair<std::string, bool>>{{"COUNT(*)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_cols));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 14);
}

TEST_F(ClickBenchTest, Query15) {
    std::set<std::string> cols = {"UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountState>(), "UserID");
    std::vector<std::string> res_names = {"COUNT(*)"};
    std::vector<std::string> group_cols = {"UserID"};
    auto agg_ptr = std::make_unique<AggregationOperator>(
        std::move(scan_ptr), std::move(states), std::move(res_names), std::move(group_cols));

    auto sort_cols = std::vector<std::pair<std::string, bool>>{{"COUNT(*)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_cols));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 15);
}

TEST_F(ClickBenchTest, Query16) {
    std::set<std::string> cols = {"UserID", "SearchPhrase"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountState>(), "UserID");
    std::vector<std::string> res_names = {"COUNT(*)"};
    std::vector<std::string> group_cols = {"UserID", "SearchPhrase"};
    auto agg_ptr = std::make_unique<AggregationOperator>(
        std::move(scan_ptr), std::move(states), std::move(res_names), std::move(group_cols));

    auto sort_cols = std::vector<std::pair<std::string, bool>>{{"COUNT(*)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_cols));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 16);
}

TEST_F(ClickBenchTest, Query17) {
    std::set<std::string> cols = {"UserID", "SearchPhrase"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountState>(), "UserID");
    std::vector<std::string> res_names = {"COUNT(*)"};
    std::vector<std::string> group_cols = {"UserID", "SearchPhrase"};
    auto agg_ptr = std::make_unique<AggregationOperator>(
        std::move(scan_ptr), std::move(states), std::move(res_names), std::move(group_cols));

    auto limit_ptr = std::make_unique<LimitOperator>(std::move(agg_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 17);
}

// SKIP 18 BECOUSE OF EXTRACT FUNCTION, WHICH IS NOT IMPLEMENTED YET

TEST_F(ClickBenchTest, Query19) {
    std::set<std::string> cols = {"UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr_ptr = std::make_unique<CompareExpression<std::equal_to<int64_t>, int64_t>>(
        "UserID", static_cast<int64_t>(435090932899640449));
    auto filter_ptr = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr_ptr));

    ExecuteAndVerify(std::move(filter_ptr), 19);
}

TEST_F(ClickBenchTest, Query20) {
    std::set<std::string> cols = {"URL"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr_ptr = std::make_unique<CompareExpression<ContainsOp, std::string>>("URL", "google");
    auto filter_ptr = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr_ptr));

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<CountState>(), "URL");
    std::vector<std::string> res_names = {"COUNT(*)"};
    auto agg_ptr = std::make_unique<AggregationOperator>(std::move(filter_ptr), std::move(states),
                                                         std::move(res_names));

    ExecuteAndVerify(std::move(agg_ptr), 20);
}

TEST_F(ClickBenchTest, Query21) {
    std::set<std::string> cols = {"URL", "SearchPhrase"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr1 = std::make_unique<CompareExpression<std::not_equal_to<std::string>, std::string>>(
        "SearchPhrase", "");
    auto filter1 = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr1));

    auto expr2 = std::make_unique<CompareExpression<ContainsOp, std::string>>("URL", "google");
    auto filter2 = std::make_unique<FilterOperator>(std::move(filter1), std::move(expr2));

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<MinState<std::string>>(Type::string), "URL");
    states.emplace_back(std::make_unique<CountState>(), "SearchPhrase");
    std::vector<std::string> res_names = {"MIN(URL)", "COUNT(*)"};
    std::vector<std::string> group_cols = {"SearchPhrase"};
    auto agg_ptr = std::make_unique<AggregationOperator>(
        std::move(filter2), std::move(states), std::move(res_names), std::move(group_cols));

    auto sort_cols = std::vector<std::pair<std::string, bool>>{{"COUNT(*)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_cols));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 21);
}

TEST_F(ClickBenchTest, Query22) {
    std::set<std::string> cols = {"Title", "URL", "SearchPhrase", "UserID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr1 = std::make_unique<CompareExpression<std::not_equal_to<std::string>, std::string>>(
        "SearchPhrase", "");
    auto filter1 = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr1));

    auto expr2 = std::make_unique<CompareExpression<ContainsOp, std::string>>("Title", "Google");
    auto filter2 = std::make_unique<FilterOperator>(std::move(filter1), std::move(expr2));

    auto expr3 = std::make_unique<CompareExpression<NotContainsOp, std::string>>("URL", ".google.");
    auto filter3 = std::make_unique<FilterOperator>(std::move(filter2), std::move(expr3));

    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states;
    states.emplace_back(std::make_unique<MinState<std::string>>(Type::string), "URL");
    states.emplace_back(std::make_unique<MinState<std::string>>(Type::string), "Title");
    states.emplace_back(std::make_unique<CountState>(), "SearchPhrase");
    states.emplace_back(std::make_unique<CountDistinctState<int64_t>>(Type::int64), "UserID");
    std::vector<std::string> res_names = {"MIN(URL)", "MIN(Title)", "COUNT(*)",
                                          "COUNT(DISTINCT UserID)"};
    std::vector<std::string> group_cols = {"SearchPhrase"};
    auto agg_ptr = std::make_unique<AggregationOperator>(
        std::move(filter3), std::move(states), std::move(res_names), std::move(group_cols));

    auto sort_cols = std::vector<std::pair<std::string, bool>>{{"COUNT(*)", true}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(agg_ptr), std::move(sort_cols));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 22);
}

TEST_F(ClickBenchTest, Query23) {
    // SELECT * FROM hits WHERE URL LIKE '%google%' ORDER BY EventTime LIMIT 10;
    std::set<std::string> cols;
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    auto expr = std::make_unique<CompareExpression<ContainsOp, std::string>>("URL", "google");
    auto filter_ptr = std::make_unique<FilterOperator>(std::move(scan_ptr), std::move(expr));

    auto sort_cols = std::vector<std::pair<std::string, bool>>{{"EventTime", false}};
    auto sort_ptr = std::make_unique<SortOperator>(std::move(filter_ptr), std::move(sort_cols));
    auto limit_ptr = std::make_unique<LimitOperator>(std::move(sort_ptr), 10);

    ExecuteAndVerify(std::move(limit_ptr), 23);
}