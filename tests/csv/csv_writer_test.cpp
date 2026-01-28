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
    CSVWriter writer(test_csv_str);
    EXPECT_TRUE(writer.Open());
    EXPECT_FALSE(writer.Crashed());
}

TEST_F(CSVWriterTest, WriteBeforeOpen) {
    CSVWriter writer(test_csv_str);
    EXPECT_FALSE(writer.WriteStr({"a", "b", "c"}, false));
}

TEST_F(CSVWriterTest, CrashedInitiallyFalse) {
    CSVWriter writer(test_csv_str);
    EXPECT_FALSE(writer.Crashed());
}

TEST_F(CSVWriterTest, FlushReturnsTrueWhenOpen) {
    CSVWriter writer(test_csv_str);
    ASSERT_TRUE(writer.Open());

    EXPECT_TRUE(writer.Flush());
    EXPECT_FALSE(writer.Crashed());
}

TEST_F(CSVWriterTest, FlushReturnsFalseBeforeOpen) {
    CSVWriter writer(test_csv_str);

    EXPECT_FALSE(writer.Flush());
    EXPECT_TRUE(writer.Crashed());
}

TEST_F(CSVWriterTest, WriteSimpleRow) {
    CSVWriter writer(test_csv_str);
    ASSERT_TRUE(writer.Open());

    EXPECT_TRUE(writer.WriteStr({"id", "name", "value"}, true));

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "id,name,value\n");
}

TEST_F(CSVWriterTest, WriteWithoutFlushThenManualFlush) {
    CSVWriter writer(test_csv_str);
    ASSERT_TRUE(writer.Open());

    EXPECT_TRUE(writer.WriteStr({"a", "b", "c"}, false));
    EXPECT_TRUE(writer.Flush());

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "a,b,c\n");
}

TEST_F(CSVWriterTest, WriteMultipleRows) {
    CSVWriter writer(test_csv_str);
    ASSERT_TRUE(writer.Open());

    EXPECT_TRUE(writer.WriteStr({"a", "b", "c"}, false));
    EXPECT_TRUE(writer.WriteStr({"1", "2", "3"}, false));
    EXPECT_TRUE(writer.WriteStr({"x", "y", "z"}, true));

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "a,b,c\n1,2,3\nx,y,z\n");
}

TEST_F(CSVWriterTest, WriteFieldWithComma) {
    CSVWriter writer(test_csv_str);
    ASSERT_TRUE(writer.Open());

    EXPECT_TRUE(writer.WriteStr({"normal", "field, with comma", "end"}, true));

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,\"field, with comma\",end\n");
}

TEST_F(CSVWriterTest, WriteFieldWithQuotes) {
    CSVWriter writer(test_csv_str);
    ASSERT_TRUE(writer.Open());

    EXPECT_TRUE(writer.WriteStr({"normal", "say \"hello\" world", "end"}, true));

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,\"say \"\"hello\"\" world\",end\n");
}

TEST_F(CSVWriterTest, WriteFieldWithNewline) {
    CSVWriter writer(test_csv_str);
    ASSERT_TRUE(writer.Open());

    EXPECT_TRUE(writer.WriteStr({"normal", "line1\nline2", "end"}, true));

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,\"line1\nline2\",end\n");
}

TEST_F(CSVWriterTest, WriteFieldWithAllSpecialChars) {
    CSVWriter writer(test_csv_str);
    ASSERT_TRUE(writer.Open());

    EXPECT_TRUE(writer.WriteStr({"complex", "has \"quotes\", commas,\nand newlines", "end"}, true));

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "complex,\"has \"\"quotes\"\", commas,\nand newlines\",end\n");
}

TEST_F(CSVWriterTest, WriteFieldWithTrailingSpace) {
    CSVWriter writer(test_csv_str);
    ASSERT_TRUE(writer.Open());

    EXPECT_TRUE(writer.WriteStr({"normal", "trailing space ", "end"}, true));

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "normal,trailing space ,end\n");
}

TEST_F(CSVWriterTest, DestructorFlushesData) {
    {
        CSVWriter writer(test_csv_str);
        ASSERT_TRUE(writer.Open());
        writer.WriteStr({"auto", "flush"}, false);
    }

    std::string content = ReadFileContent();
    EXPECT_EQ(content, "auto,flush\n");
}

TEST_F(CSVWriterTest, CrashedAfterFlushBeforeOpen) {
    CSVWriter writer(test_csv_str);

    EXPECT_FALSE(writer.Crashed());
    EXPECT_FALSE(writer.Flush());
    EXPECT_TRUE(writer.Crashed());

    EXPECT_FALSE(writer.WriteStr({"a"}, true));
}