#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <map>

#include "transform/columnar_to_csv.h"
#include "transform/csv_to_columnar.h"

class TransformTest : public ::testing::Test {
protected:
    std::filesystem::path tmp_dir;
    std::filesystem::path csv_file;
    std::filesystem::path scheme_file;
    std::filesystem::path columnar_file;
    std::filesystem::path out_csv_file;
    std::filesystem::path out_scheme_file;

    void SetUp() override {
        tmp_dir = std::filesystem::temp_directory_path();
        csv_file = tmp_dir / "test_data.csv";
        scheme_file = tmp_dir / "test_scheme.csv";
        columnar_file = tmp_dir / "test_data.br";
        out_csv_file = tmp_dir / "test_data_res.csv";
        out_scheme_file = tmp_dir / "test_scheme_res.csv";

        std::ofstream csv(csv_file);
        csv << "1,2,first,4,\"2013-07-14 20:38:47\",\"2013-07-15\"\n";
        csv << "5,1,second,2,\"2013-03-18 00:59:47\",\"2025-01-15\"\n";
        csv << "8,17,third,2,\"2000-07-14 20:55:59\",\"1972-12-01\"\n";

        std::ofstream scheme(scheme_file);
        scheme << "a,int64\n";
        scheme << "b,int32\n";
        scheme << "name123,string\n";
        scheme << "d,int16\n";
        scheme << "e,timestamp\n";
        scheme << "f,date\n";
    }

    void TearDown() override {
        std::filesystem::remove(csv_file);
        std::filesystem::remove(scheme_file);
        std::filesystem::remove(columnar_file);
        std::filesystem::remove(out_csv_file);
        std::filesystem::remove(out_scheme_file);
    }

    static bool CompareCSV(const std::filesystem::path& file1, const std::filesystem::path& file2) {
        std::ifstream fin1(file1);
        std::ifstream fin2(file2);
        std::map<char, int> cnt1, cnt2;
        while (fin1.peek() != EOF) {
            cnt1[fin1.get()]++;
        }
        while (fin2.peek() != EOF) {
            cnt2[fin2.get()]++;
        }
        cnt1.erase('\n');
        cnt1.erase('"');
        cnt2.erase('\n');
        cnt2.erase('"');
        return cnt1 == cnt2;
    }
};

TEST_F(TransformTest, CsvToColumnarAndBack_DefaultBatch) {
    {
        CSVToColumnarTransformer to_col(csv_file.string(), scheme_file.string(),
                                        columnar_file.string());
        auto res = to_col.Transform();
        ASSERT_TRUE(res.has_value()) << res.error();
    }
    {
        ColumnarToCSVTransformer to_csv(columnar_file.string(), out_scheme_file.string(),
                                        out_csv_file.string());
        auto res = to_csv.Transform();
        ASSERT_TRUE(res.has_value()) << res.error();
    }

    EXPECT_TRUE(CompareCSV(csv_file, out_csv_file));
    EXPECT_TRUE(CompareCSV(scheme_file, out_scheme_file));
}

TEST_F(TransformTest, CsvToColumnarAndBack_SmallBatch) {
    {
        CSVToColumnarTransformer to_col(csv_file.string(), scheme_file.string(),
                                        columnar_file.string(), 1);
        auto res = to_col.Transform();
        ASSERT_TRUE(res.has_value()) << res.error();
    }
    {
        ColumnarToCSVTransformer to_csv(columnar_file.string(), out_scheme_file.string(),
                                        out_csv_file.string());
        auto res = to_csv.Transform();
        ASSERT_TRUE(res.has_value()) << res.error();
    }

    EXPECT_TRUE(CompareCSV(csv_file, out_csv_file));
    EXPECT_TRUE(CompareCSV(scheme_file, out_scheme_file));
}
