#include "csv_reader.h"

#include <gtest/gtest.h>

#include <filesystem>

class CSVReaderTest : public ::testing::Test {
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

    void SkipRows(CSVReader& reader, int n) {
        for (int i = 0; i < n; i++) {
            reader.NextStr();
        }
    }
};

TEST_F(CSVReaderTest, OpenValidFile) {
    CSVReader reader(test_csv_str);
    EXPECT_TRUE(reader.Open());
}

TEST_F(CSVReaderTest, OpenInvalidFile) {
    CSVReader reader("aboba.csv");
    EXPECT_FALSE(reader.Open());
}

TEST_F(CSVReaderTest, HasNextBeforeOpen) {
    CSVReader reader(test_csv_str);
    EXPECT_FALSE(reader.HasNext());
}

TEST_F(CSVReaderTest, HasNextAfterOpen) {
    CSVReader reader(test_csv_str);
    ASSERT_TRUE(reader.Open());
    EXPECT_TRUE(reader.HasNext());
}

TEST_F(CSVReaderTest, ReadHeader) {
    CSVReader reader(test_csv_str);
    ASSERT_TRUE(reader.Open());

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value());

    auto& fields = result.value();
    ASSERT_EQ(fields.size(), 3);
    EXPECT_EQ(fields[0], "id");
    EXPECT_EQ(fields[1], "name");
    EXPECT_EQ(fields[2], "description");
}

TEST_F(CSVReaderTest, ReadQuotedField) {
    CSVReader reader(test_csv_str);
    ASSERT_TRUE(reader.Open());
    SkipRows(reader, 1);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value());

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "1");
    EXPECT_EQ(fields[1], "Quoted field");
    EXPECT_EQ(fields[2], "SimpleText");
}

TEST_F(CSVReaderTest, ReadFieldWithCommaAndTrailingSpace) {
    CSVReader reader(test_csv_str);
    ASSERT_TRUE(reader.Open());
    SkipRows(reader, 2);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value());

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "2");
    EXPECT_EQ(fields[2], "Field with, comma inside and space ended ");
}

TEST_F(CSVReaderTest, ReadMultilineField) {
    CSVReader reader(test_csv_str);
    ASSERT_TRUE(reader.Open());
    SkipRows(reader, 3);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value());

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "3");
    EXPECT_EQ(fields[2], "This field contains\na line break inside quotes");
}

TEST_F(CSVReaderTest, ReadEscapedQuotes) {
    CSVReader reader(test_csv_str);
    ASSERT_TRUE(reader.Open());
    SkipRows(reader, 4);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value());

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "4");
    EXPECT_EQ(fields[2], "String \"with\" quotes");
}

TEST_F(CSVReaderTest, ReadEmptyField) {
    CSVReader reader(test_csv_str);
    ASSERT_TRUE(reader.Open());
    SkipRows(reader, 5);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value());

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "5");
    EXPECT_EQ(fields[1], "Empty");
    EXPECT_EQ(fields[2], "");
}

TEST_F(CSVReaderTest, ReadComplexField) {
    CSVReader reader(test_csv_str);
    ASSERT_TRUE(reader.Open());
    SkipRows(reader, 6);

    auto result = reader.NextStr();
    ASSERT_TRUE(result.has_value());

    auto& fields = result.value();
    EXPECT_EQ(fields[0], "6");
    EXPECT_EQ(fields[2],
              "Final entry with all features: \"quotes\", commas, and newlines\ncontinued here");
}

TEST_F(CSVReaderTest, HasNextReturnsFalseAfterLastRow) {
    CSVReader reader(test_csv_str);
    ASSERT_TRUE(reader.Open());

    while (reader.HasNext()) {
        reader.NextStr();
    }

    EXPECT_FALSE(reader.HasNext());
    EXPECT_FALSE(reader.NextStr().has_value());
}
