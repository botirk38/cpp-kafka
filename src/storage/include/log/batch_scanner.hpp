#pragma once

#include "io/binary_cursor.hpp"
#include "storage_types.hpp"
#include <fstream>
#include <optional>
#include <vector>

namespace storage::log {

class BatchScanner {
public:
  explicit BatchScanner(std::ifstream &file);

  // Scan all record batches from file, return raw bytes for each
  std::vector<RecordBatchBytes> scanAll();

private:
  std::optional<RecordBatchBytes> scanOne();

  io::BinaryCursor cursor_;
};

} // namespace storage::log
