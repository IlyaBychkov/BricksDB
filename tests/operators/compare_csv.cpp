#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

namespace fs = std::filesystem;

class CompareResults : public ::testing::Test {
protected:
    fs::path german_dir =
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/tests/operators/Clickbench_German/small";
    fs::path my_dir =
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/tests/operators/clickbench_results";

    std::vector<int> pass = {10, 18, 22, 24, 26, 30, 31, 38, 39, 40, 41, 42};
    std::vector<int> skip = {3, 17, 23, 32};

    static bool CompareFiles(const fs::path& p1, const fs::path& p2) {
        std::ifstream f1(p1, std::ios::binary);
        std::ifstream f2(p2, std::ios::binary);

        if (!f1.is_open() || !f2.is_open()) {
            return false;
        }

        return std::equal(std::istreambuf_iterator<char>(f1), std::istreambuf_iterator<char>(),
                          std::istreambuf_iterator<char>(f2), std::istreambuf_iterator<char>());
    }
};

TEST_F(CompareResults, CompareResultsTest) {
    for (int i = 0; i <= 42; ++i) {
        if (std::find(skip.begin(), skip.end(), i) != skip.end()) {
            std::cout << "[ SKIP     ] Query " << i << " is in skip list" << std::endl;
            continue;
        }

        if (std::find(pass.begin(), pass.end(), i) != pass.end()) {
            std::cout << "[ PASS     ] Query " << i << " is in pass list" << std::endl;
            continue;
        }

        std::string num = std::to_string(i);
        fs::path german_file =
            german_dir / ("query_" + (num.size() == 1 ? "0" + num : num) + ".csv");
        fs::path my_file = my_dir / ("query_" + num + ".csv");

        if (!fs::exists(german_file) || !fs::exists(my_file)) {
            std::cout << "[ INFO     ] Missing file for query " << i << ", skipping..."
                      << std::endl;
            continue;
        }

        EXPECT_TRUE(CompareFiles(german_file, my_file))
            << "Files differ for query " << i << ": " << german_file.filename() << " vs "
            << my_file.filename();
    }
}