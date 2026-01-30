#include "transform/csv_to_columnar.h"

#include <iostream>

int main() {
    CSVToColumnarTransformer transformer(
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/test_files/data.csv",
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/test_files/scheme.csv",
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/test_files/data.br", 1);
    if (!transformer.Transform()) {
        std::cout << "Transformation failed" << std::endl;
    } else {
        std::cout << "Transformation succeeded" << std::endl;
    }
}