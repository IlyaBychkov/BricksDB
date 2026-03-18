#include "transform/columnar_to_csv.h"

#include <iostream>

int main() {
    ColumnarToCSVTransformer transformer(
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/test_files/data.br",
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/test_files/data_scheme.csv",
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/test_files/data_res.csv");
    auto res = transformer.Transform();
    if (!res) {
        std::cout << "Transformation failed: " << res.error() << std::endl;
    } else {
        std::cout << "Transformation succeeded" << std::endl;
    }
}