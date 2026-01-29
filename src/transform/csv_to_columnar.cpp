#include "transform/csv_to_columnar.h"

#include "transform/metadata.h"

CSVToColumnarTransformer::CSVToColumnarTransformer(const std::string& csv_filename,
                                                   const std::string& scheme_filename,
                                                   int64_t batch_max_size)
    : csv_filename_(csv_filename),
      scheme_filename_(scheme_filename),
      batch_max_size_(batch_max_size) {
    columnar_filename_ = csv_filename_.substr(0, csv_filename_.find_last_of('.')) + ".br";
}

CSVToColumnarTransformer::~CSVToColumnarTransformer() {
    if (fout_.is_open()) {
        fout_.close();
    }
}

bool CSVToColumnarTransformer::Prepare() {
    auto tmp = CreateCSVBatcher(csv_filename_, scheme_filename_, batch_max_size_);
    if (!tmp.has_value()) {
        return false;
    }
    batcher_ = std::move(tmp.value());
    fout_.open(columnar_filename_, std::ios::binary);
    return fout_.is_open();
}

bool CSVToColumnarTransformer::Transform() {
    if (!Prepare()) {
        return false;
    }

    Metadata metadata(batcher_.GetScheme());

    while (batcher_.HasNextBatch()) {
        auto tmp2 = batcher_.NextBatch();
        if (!tmp2.has_value()) {
            return false;
        }
        Batch batch = std::move(tmp2.value());
        metadata.AddRowGroup(fout_.tellp(), batch.RowsCnt());
        if (!WriteBatch(batch)) {
            return false;
        }
    }

    metadata.WriteToFile(fout_);

    return !batcher_.IsCrashed();
}

bool CSVToColumnarTransformer::WriteBatch(const Batch& batch) {
    for (const auto& column : batch.GetAllColumns()) {
        if (!column.WriteToFile(fout_)) {
            return false;
        }
    }

    return fout_.good();
}