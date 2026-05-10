#include "transform/columnar_to_csv.h"

#include <iostream>

int main() {
    ColumnarToCSVTransformer transformer(
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/hits_files/hits_sample.br",
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/hits_files/hits_scheme_res.csv",
        "/home/ilya-bychkov/VsCodeProjects/BricksDB/hits_files/hits_sample_res.csv");
    auto res = transformer.Transform();
    if (!res) {
        std::cout << "Transformation failed: " << res.error() << std::endl;
    } else {
        std::cout << "Transformation succeeded" << std::endl;
    }
}