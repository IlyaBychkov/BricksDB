#include "transform/columnar_to_csv.h"

#include "scheme/batch.h"
#include "scheme/column.h"
#include "transform/metadata.h"

ColumnarToCSVTransformer::ColumnarToCSVTransformer(const std::string& columnar_filename,
                                                   const std::string& scheme_filename,
                                                   const std::string& csv_filename)
    : columnar_filename_(columnar_filename),
      scheme_filename_(scheme_filename),
      csv_filename_(csv_filename) {
}

ColumnarToCSVTransformer::~ColumnarToCSVTransformer() {
    if (fin_.is_open()) {
        fin_.close();
    }
}

bool ColumnarToCSVTransformer::Prepare() {
    fin_.open(columnar_filename_, std::ios::binary);
    if (!fin_.is_open()) {
        return false;
    }

    auto tmp_csv = CreateCSVWriter(csv_filename_);
    if (!tmp_csv.has_value()) {
        return false;
    }
    csv_out_ = std::move(tmp_csv.value());

    return true;
}

bool ColumnarToCSVTransformer::Transform() {
    if (!Prepare()) {
        return false;
    }

    Metadata metadata;
    if (!metadata.ReadFromFile(fin_)) {
        return false;
    }

    Scheme scheme = metadata.GetScheme();
    scheme.WriteToFile(scheme_filename_);

    std::vector<int64_t>& offsets = metadata.GetOffsets();
    std::vector<int64_t>& rows = metadata.GetRowsCnt();

    for (size_t i = 0; i < offsets.size(); ++i) {
        fin_.seekg(offsets[i], std::ios::beg);
        Batch batch(scheme, fin_, rows[i]);
        WriteBatchToCSV(batch);
    }

    return !csv_out_.IsCrashed();
}

bool ColumnarToCSVTransformer::WriteBatchToCSV(const Batch& batch) {
    for (size_t i = 0; i < batch.RowsCnt(); ++i) {
        std::vector<std::string> row;
        for (size_t c = 0; c < batch.ColumnsCnt(); ++c) {
            if (batch.GetColumnType(c) == Type::int64) {
                const auto& val = batch.GetColumn(c).GetValue<int64_t>(i);
                row.push_back(std::to_string(val));
            } else if (batch.GetColumnType(c) == Type::string) {
                const auto& val = batch.GetColumn(c).GetValue<std::string>(i);
                row.push_back(val);
            } else {
                return false;
            }
        }
        if (!csv_out_.WriteRow(row)) {
            return false;
        }
    }
    return !csv_out_.IsCrashed();
}