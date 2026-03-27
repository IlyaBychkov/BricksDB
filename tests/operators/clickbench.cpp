#include <gtest/gtest.h>

#include <filesystem>

#include "operators/aggregation_operator.h"
#include "operators/scan_operator.h"
#include "scheme/batch.h"
#include "scheme/scheme.h"

void WriteResult(const Batch& batch, const std::string& filename,
                 const std::string& scheme_filename) {
    auto wres = WriteBatchToCSV(batch, filename);
    ASSERT_TRUE(wres.has_value()) << "Failed to write batch: " << wres.error();

    auto sres = WriteSchemeToCSV(batch.GetScheme(), scheme_filename);
    ASSERT_TRUE(sres.has_value()) << "Failed to write scheme: " << sres.error();
}

TEST(ClickBench, query0) {
    std::filesystem::path repo_root("/home/ilya-bychkov/VsCodeProjects/BricksDB");
    std::filesystem::path hits_file = repo_root / "hits_files" / "hits_sample.br";

    std::set<std::string> cols = {"WatchID"};
    auto scan_ptr = std::make_unique<ScanOperator>(hits_file.string(), cols);

    std::vector<std::pair<AggregationType, std::string>> aggs = {
        {AggregationType::COUNT, "WatchID"}};
    AggregationOperator agg_op(std::move(scan_ptr), aggs);

    auto res_batch = agg_op.Next();
    ASSERT_TRUE(res_batch.has_value());

    std::string out_dir = (repo_root / "tests" / "operators" / "clickbench_results").string();
    WriteResult(*res_batch, out_dir + "/0ans.csv", out_dir + "/0scheme.csv");
}
