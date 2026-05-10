#include "transform/csv_to_columnar.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Wrong count of parametrs." << std::endl;
        return 1;
    }
    std::string input_csv = argv[1];
    std::string scheme_csv = argv[2];
    std::string output_bricks = argv[3];
    int64_t buffer_size = 1024ll * 1024 * 512;

    std::cout << "Starting transformation..." << std::endl;
    std::cout << "Input: " << input_csv << "\nScheme: " << scheme_csv
              << "\nOutput: " << output_bricks << std::endl;

    CSVToColumnarTransformer transformer(input_csv, scheme_csv, output_bricks, buffer_size);
    auto res = transformer.Transform();
    if (!res) {
        std::cerr << "Transformation failed: " << res.error() << std::endl;
        return 1;
    } else {
        std::cout << "Transformation succeeded" << std::endl;
        return 0;
    }
}