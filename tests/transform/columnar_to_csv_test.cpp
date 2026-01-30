#include "transform/columnar_to_csv.h"

#include <iostream>

int main() {
    ColumnarToCSVTransformer transformer(
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/test_files/data.br",
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/test_files/data_scheme.csv",
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/test_files/data_res.csv");
    if (!transformer.Transform()) {
        std::cout << "Transformation failed" << std::endl;
    } else {
        std::cout << "Transformation succeeded" << std::endl;
    }
}