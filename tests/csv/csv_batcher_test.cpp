#include "csv/csv_batcher.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

class CsvBatcherTest : public ::testing::Test {
protected:
    std::filesystem::path test_csv_file;
    std::filesystem::path test_schema_file;

    void SetUp() override {
        test_csv_file = std::filesystem::temp_directory_path() / "test_batcher_data.csv";
        test_schema_file = std::filesystem::temp_directory_path() / "test_batcher_schema.csv";

        std::ofstream csv(test_csv_file);
        csv << "1,Alpha,First\n";
        csv << "2,Beta,Second\n";
        csv << "3,Gamma,Third\n";

        std::ofstream schema(test_schema_file);
        schema << "id,int64\n";
        schema << "name,string\n";
        schema << "description,string\n";
    }

    void TearDown() override {
        std::filesystem::remove(test_csv_file);
        std::filesystem::remove(test_schema_file);
    }
};

TEST_F(CsvBatcherTest, CreateValidBatcher) {
    {
        auto res = CreateCsvReader(test_schema_file.string());
        ASSERT_TRUE(res.has_value()) << res.error();
        CsvReader r = std::move(res.value());
    }

    auto res = CreateCsvBatcher(test_csv_file.string(), test_schema_file.string());
    ASSERT_TRUE(res.has_value()) << res.error();

    CsvBatcher batcher = std::move(res.value());
    EXPECT_TRUE(batcher.HasNextBatch());
}

TEST_F(CsvBatcherTest, SingleBatchContainsAllRows) {
    auto res = CreateCsvBatcher(test_csv_file.string(), test_schema_file.string());
    ASSERT_TRUE(res.has_value()) << res.error();

    CsvBatcher batcher = std::move(res.value());
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

TEST_F(CsvBatcherTest, BatchesSplitBySize) {
    auto res = CreateCsvBatcher(test_csv_file.string(), test_schema_file.string(), 1);
    ASSERT_TRUE(res.has_value()) << res.error();

    CsvBatcher batcher = std::move(res.value());

    int total_rows = 0;
    while (batcher.HasNextBatch()) {
        auto bres = batcher.NextBatch();
        ASSERT_TRUE(bres.has_value()) << bres.error();
        Batch b = std::move(bres.value());
        total_rows += b.RowsCnt() == 1;
    }

    EXPECT_EQ(total_rows, 3);
}
