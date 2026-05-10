#include "csv/csv_batcher.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

class CSVBatcherTest : public ::testing::Test {
protected:
    std::filesystem::path test_csv_file;
    std::filesystem::path test_scheme_file;

    void SetUp() override {
        test_csv_file = std::filesystem::temp_directory_path() / "test_batcher_data.csv";
        test_scheme_file = std::filesystem::temp_directory_path() / "test_batcher_scheme.csv";

        std::ofstream csv(test_csv_file);
        csv << "1,Alpha,First\n";
        csv << "2,Beta,Second\n";
        csv << "3,Gamma,Third\n";

        std::ofstream scheme(test_scheme_file);
        scheme << "id,int64\n";
        scheme << "name,string\n";
        scheme << "description,string\n";
    }

    void TearDown() override {
        std::filesystem::remove(test_csv_file);
        std::filesystem::remove(test_scheme_file);
    }
};

TEST_F(CSVBatcherTest, CreateValidBatcher) {
    {
        auto res = CreateCSVReader(test_scheme_file.string());
        ASSERT_TRUE(res.has_value()) << res.error();
        CSVReader r = std::move(res.value());
    }

    auto res = CreateCSVBatcher(test_csv_file.string(), test_scheme_file.string());
    ASSERT_TRUE(res.has_value()) << res.error();

    CSVBatcher batcher = std::move(res.value());
    EXPECT_FALSE(batcher.IsCrashed());
    EXPECT_TRUE(batcher.HasNextBatch());
}

TEST_F(CSVBatcherTest, SingleBatchContainsAllRows) {
    auto res = CreateCSVBatcher(test_csv_file.string(), test_scheme_file.string());
    ASSERT_TRUE(res.has_value()) << res.error();

    CSVBatcher batcher = std::move(res.value());
    auto batch_res = batcher.NextBatch();
    ASSERT_TRUE(batch_res.has_value()) << batch_res.error();
    Batch batch = std::move(batch_res.value());

    EXPECT_EQ(batch.ColumnsCnt(), 3);
    EXPECT_EQ(batch.GetColumnType(0), Type::int64);
    EXPECT_EQ(batch.GetColumnType(1), Type::string);
    EXPECT_EQ(batch.GetColumnType(2), Type::string);
    EXPECT_EQ(batch.GetColumnName(0), "id");
    EXPECT_EQ(batch.GetColumnName(1), "name");
    EXPECT_EQ(batch.GetColumnName(2), "description");

    {
        const auto& vec = batch.GetColumn(0).GetVector<int64_t>();
        EXPECT_EQ(vec.size(), 3);
        EXPECT_EQ(vec[0], 1);
        EXPECT_EQ(vec[1], 2);
        EXPECT_EQ(vec[2], 3);
    }
    {
        const auto& vec = batch.GetColumn(1).GetVector<std::string>();
        EXPECT_EQ(vec.size(), 3);
        EXPECT_EQ(vec[0], "Alpha");
        EXPECT_EQ(vec[1], "Beta");
        EXPECT_EQ(vec[2], "Gamma");
    }
    {
        const auto& vec = batch.GetColumn(2).GetVector<std::string>();
        EXPECT_EQ(vec.size(), 3);
        EXPECT_EQ(vec[0], "First");
        EXPECT_EQ(vec[1], "Second");
        EXPECT_EQ(vec[2], "Third");
    }
}

TEST_F(CSVBatcherTest, BatchesSplitBySize) {
    auto res = CreateCSVBatcher(test_csv_file.string(), test_scheme_file.string(), 1);
    ASSERT_TRUE(res.has_value()) << res.error();

    CSVBatcher batcher = std::move(res.value());

    int total_rows = 0;
    while (batcher.HasNextBatch()) {
        auto bres = batcher.NextBatch();
        ASSERT_TRUE(bres.has_value()) << bres.error();
        Batch b = std::move(bres.value());
        total_rows += b.RowsCnt() == 1;
    }

    EXPECT_EQ(total_rows, 3);
}
