#include "csv_reader.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

class CsvReaderTest : public ::testing::Test {
protected:
    std::filesystem::path test_csv_file;
    std::string test_csv_str;

    void SetUp() override {
        test_csv_file = std::filesystem::temp_directory_path() / "test_data.csv";
        test_csv_str = test_csv_file.string();

        std::ofstream file(test_csv_file);
        file << R"(id,name,description
1,"Quoted field",SimpleText
2,SimpleText,"Field with, comma inside and space ended "
3,SimpleText,"This field contains
a line break inside quotes"
4,SimpleText,"String ""with"" quotes"
5,Empty,""
6,Last,"Final entry with all features: ""quotes"", commas, and newlines
continued here")";
    }

    void TearDown() override {
        std::filesystem::remove(test_csv_file);
    }

    void SkipRows(CsvReader& reader, int n) {
        for (int i = 0; i < n; i++) {
            reader.NextStr();
        }
    }
};

TEST_F(CsvReaderTest, OpenValidFile) {
    auto tmp = CreateCsvReader(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
}

TEST_F(CsvReaderTest, OpenInvalidFile) {
    auto tmp = CreateCsvReader("aboba.csv");
    EXPECT_FALSE(tmp.has_value());
}

TEST_F(CsvReaderTest, HasNextAfterCreate) {
    auto tmp = CreateCsvReader(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvReader reader = std::move(tmp.value());
    EXPECT_TRUE(reader.HasNext());
}

TEST_F(CsvReaderTest, ReadHeader) {
    auto tmp = CreateCsvReader(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvReader reader = std::move(tmp.value());

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value()) << result.error();

    auto& fields = result.value();
    ASSERT_EQ(fields.size(), 3);
    EXPECT_EQ(fields[0], "id");
    EXPECT_EQ(fields[1], "name");
    EXPECT_EQ(fields[2], "description");
}

TEST_F(CsvReaderTest, ReadQuotedField) {
    auto tmp = CreateCsvReader(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvReader reader = std::move(tmp.value());
    SkipRows(reader, 1);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value()) << result.error();

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "1");
    EXPECT_EQ(fields[1], "Quoted field");
    EXPECT_EQ(fields[2], "SimpleText");
}

TEST_F(CsvReaderTest, ReadFieldWithCommaAndTrailingSpace) {
    auto tmp = CreateCsvReader(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvReader reader = std::move(tmp.value());
    SkipRows(reader, 2);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value()) << result.error();

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "2");
    EXPECT_EQ(fields[2], "Field with, comma inside and space ended ");
}

TEST_F(CsvReaderTest, ReadMultilineField) {
    auto tmp = CreateCsvReader(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvReader reader = std::move(tmp.value());
    SkipRows(reader, 3);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value()) << result.error();

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "3");
    EXPECT_EQ(fields[2], "This field contains\na line break inside quotes");
}

TEST_F(CsvReaderTest, ReadEscapedQuotes) {
    auto tmp = CreateCsvReader(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvReader reader = std::move(tmp.value());
    SkipRows(reader, 4);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value()) << result.error();

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "4");
    EXPECT_EQ(fields[2], "String \"with\" quotes");
}

TEST_F(CsvReaderTest, ReadEmptyField) {
    auto tmp = CreateCsvReader(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvReader reader = std::move(tmp.value());
    SkipRows(reader, 5);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value()) << result.error();

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "5");
    EXPECT_EQ(fields[1], "Empty");
    EXPECT_EQ(fields[2], "");
}

TEST_F(CsvReaderTest, ReadComplexField) {
    auto tmp = CreateCsvReader(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvReader reader = std::move(tmp.value());
    SkipRows(reader, 6);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value()) << result.error();

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "6");
    EXPECT_EQ(fields[2],
              "Final entry with all features: \"quotes\", commas, and newlines\ncontinued here");
}

TEST_F(CsvReaderTest, HasNextReturnsFalseAfterLastRow) {
    std::filesystem::path test_schema_file =
        std::filesystem::temp_directory_path() / "test_batcher_schema.csv";

    std::ofstream schema(test_schema_file);
    schema << "id,int64\n";
    schema << "name,string\n";
    schema << "description,string\n";
    schema.flush();

    auto tmp = CreateCsvReader(test_schema_file.string());
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvReader reader = std::move(tmp.value());

    EXPECT_TRUE(reader.HasNext());
    reader.NextStr();
    EXPECT_TRUE(reader.HasNext());
    reader.NextStr();
    EXPECT_TRUE(reader.HasNext());
    reader.NextStr();

    EXPECT_FALSE(reader.HasNext());
    EXPECT_FALSE(reader.NextStr().has_value());

    std::filesystem::remove(test_schema_file);
}
