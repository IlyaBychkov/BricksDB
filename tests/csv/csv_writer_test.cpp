#include "csv_writer.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

class CSVWriterTest : public ::testing::Test {
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

TEST_F(CSVWriterTest, OpenValidPath) {
    auto tmp = CreateCSVWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value());
    CSVWriter writer = std::move(tmp.value());
    ;
    EXPECT_FALSE(writer.IsCrashed());
}

TEST_F(CSVWriterTest, IsCrashedInitiallyFalse) {
    auto tmp = CreateCSVWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value());
    CSVWriter writer = std::move(tmp.value());
    EXPECT_FALSE(writer.IsCrashed());
}

TEST_F(CSVWriterTest, FlushReturnsTrueWhenOpen) {
    auto tmp = CreateCSVWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value());
    CSVWriter writer = std::move(tmp.value());

    EXPECT_TRUE(writer.Flush());
    EXPECT_FALSE(writer.IsCrashed());
}

TEST_F(CSVWriterTest, WriteSimpleRow) {
    auto tmp = CreateCSVWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value());
    CSVWriter writer = std::move(tmp.value());

    EXPECT_TRUE(writer.WriteRow({"id", "name", "value"}, true).has_value());

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "id,name,value\n");
}

TEST_F(CSVWriterTest, WriteWithoutFlushThenManualFlush) {
    auto tmp = CreateCSVWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value());
    CSVWriter writer = std::move(tmp.value());

    EXPECT_TRUE(writer.WriteRow({"a", "b", "c"}, false).has_value());
    EXPECT_TRUE(writer.Flush());

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "a,b,c\n");
}

TEST_F(CSVWriterTest, WriteMultipleRows) {
    auto tmp = CreateCSVWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value());
    CSVWriter writer = std::move(tmp.value());

    EXPECT_TRUE(writer.WriteRow({"a", "b", "c"}, false).has_value());
    EXPECT_TRUE(writer.WriteRow({"1", "2", "3"}, false).has_value());
    EXPECT_TRUE(writer.WriteRow({"x", "y", "z"}, true).has_value());

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "a,b,c\n1,2,3\nx,y,z\n");
}

TEST_F(CSVWriterTest, WriteFieldWithComma) {
    auto tmp = CreateCSVWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value());
    CSVWriter writer = std::move(tmp.value());

    EXPECT_TRUE(writer.WriteRow({"normal", "field, with comma", "end"}, true).has_value());

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,\"field, with comma\",end\n");
}

TEST_F(CSVWriterTest, WriteFieldWithQuotes) {
    auto tmp = CreateCSVWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value());
    CSVWriter writer = std::move(tmp.value());

    EXPECT_TRUE(writer.WriteRow({"normal", "say \"hello\" world", "end"}, true).has_value());

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,\"say \"\"hello\"\" world\",end\n");
}

TEST_F(CSVWriterTest, WriteFieldWithNewline) {
    auto tmp = CreateCSVWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value());
    CSVWriter writer = std::move(tmp.value());

    EXPECT_TRUE(writer.WriteRow({"normal", "line1\nline2", "end"}, true).has_value());

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,\"line1\nline2\",end\n");
}

TEST_F(CSVWriterTest, WriteFieldWithAllSpecialChars) {
    auto tmp = CreateCSVWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value());
    CSVWriter writer = std::move(tmp.value());

    EXPECT_TRUE(writer.WriteRow({"complex", "has \"quotes\", commas,\nand newlines", "end"}, true)
                    .has_value());

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "complex,\"has \"\"quotes\"\", commas,\nand newlines\",end\n");
}

TEST_F(CSVWriterTest, WriteFieldWithTrailingSpace) {
    auto tmp = CreateCSVWriter(test_csv_str);
    ASSERT_TRUE(tmp.has_value());
    CSVWriter writer = std::move(tmp.value());

    EXPECT_TRUE(writer.WriteRow({"normal", "trailing space ", "end"}, true).has_value());

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,trailing space ,end\n");
}

TEST_F(CSVWriterTest, DestructorFlushesData) {
    {
        auto tmp = CreateCSVWriter(test_csv_str);
        ASSERT_TRUE(tmp.has_value());
        CSVWriter writer = std::move(tmp.value());

        ASSERT_TRUE(writer.WriteRow({"auto", "flush"}, false).has_value());
    }

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "auto,flush\n");
}