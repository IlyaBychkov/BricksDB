#include "csv_writer.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

class CsvWriterTest : public ::testing::Test {
protected:
    std::filesystem::path test_csv_file;
    std::string test_csv_str;

    void SetUp() override {
        test_csv_file = std::filesystem::temp_directory_path() / "test_output.csv";
        test_csv_str = test_csv_file.string();
    }

    void TearDown() override {
        std::filesystem::remove(test_csv_file);
    }

    std::string ReadFileContent() {
        std::ifstream file(test_csv_file);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

TEST_F(CsvWriterTest, OpenValidPath) {
    auto tmp = CreateCsvWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvWriter writer = std::move(tmp.value());
}

TEST_F(CsvWriterTest, IsCrashedInitiallyFalse) {
    auto tmp = CreateCsvWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvWriter writer = std::move(tmp.value());
}

TEST_F(CsvWriterTest, FlushReturnsTrueWhenOpen) {
    auto tmp = CreateCsvWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvWriter writer = std::move(tmp.value());

    EXPECT_TRUE(writer.Flush());
}

TEST_F(CsvWriterTest, WriteSimpleRow) {
    auto tmp = CreateCsvWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvWriter writer = std::move(tmp.value());

    auto res = writer.WriteRow({"id", "name", "value"}, true);
    ASSERT_TRUE(res.has_value()) << res.error();

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "id,name,value\n");
}

TEST_F(CsvWriterTest, WriteWithoutFlushThenManualFlush) {
    auto tmp = CreateCsvWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvWriter writer = std::move(tmp.value());

    auto res = writer.WriteRow({"a", "b", "c"}, false);
    ASSERT_TRUE(res.has_value()) << res.error();
    EXPECT_TRUE(writer.Flush());

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "a,b,c\n");
}

TEST_F(CsvWriterTest, WriteMultipleRows) {
    auto tmp = CreateCsvWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvWriter writer = std::move(tmp.value());

    auto r1 = writer.WriteRow({"a", "b", "c"}, false);
    ASSERT_TRUE(r1.has_value()) << r1.error();
    auto r2 = writer.WriteRow({"1", "2", "3"}, false);
    ASSERT_TRUE(r2.has_value()) << r2.error();
    auto r3 = writer.WriteRow({"x", "y", "z"}, true);
    ASSERT_TRUE(r3.has_value()) << r3.error();

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "a,b,c\n1,2,3\nx,y,z\n");
}

TEST_F(CsvWriterTest, WriteFieldWithComma) {
    auto tmp = CreateCsvWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvWriter writer = std::move(tmp.value());

    auto res = writer.WriteRow({"normal", "field, with comma", "end"}, true);
    ASSERT_TRUE(res.has_value()) << res.error();

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,\"field, with comma\",end\n");
}

TEST_F(CsvWriterTest, WriteFieldWithQuotes) {
    auto tmp = CreateCsvWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvWriter writer = std::move(tmp.value());

    auto res = writer.WriteRow({"normal", "say \"hello\" world", "end"}, true);
    ASSERT_TRUE(res.has_value()) << res.error();

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,\"say \"\"hello\"\" world\",end\n");
}

TEST_F(CsvWriterTest, WriteFieldWithNewline) {
    auto tmp = CreateCsvWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvWriter writer = std::move(tmp.value());

    auto res = writer.WriteRow({"normal", "line1\nline2", "end"}, true);
    ASSERT_TRUE(res.has_value()) << res.error();

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,\"line1\nline2\",end\n");
}

TEST_F(CsvWriterTest, WriteFieldWithAllSpecialChars) {
    auto tmp = CreateCsvWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvWriter writer = std::move(tmp.value());

    auto res = writer.WriteRow({"complex", "has \"quotes\", commas,\nand newlines", "end"}, true);
    ASSERT_TRUE(res.has_value()) << res.error();

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "complex,\"has \"\"quotes\"\", commas,\nand newlines\",end\n");
}

TEST_F(CsvWriterTest, WriteFieldWithTrailingSpace) {
    auto tmp = CreateCsvWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value()) << tmp.error();
    CsvWriter writer = std::move(tmp.value());

    auto res = writer.WriteRow({"normal", "trailing space ", "end"}, true);
    ASSERT_TRUE(res.has_value()) << res.error();

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,trailing space ,end\n");
}

TEST_F(CsvWriterTest, DestructorFlushesData) {
    {
        auto tmp = CreateCsvWriter(test_csv_str);
        ASSERT_TRUE(tmp.has_value()) << tmp.error();
        CsvWriter writer = std::move(tmp.value());

        auto res = writer.WriteRow({"auto", "flush"}, false);
        ASSERT_TRUE(res.has_value()) << res.error();
    }

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "auto,flush\n");
}
