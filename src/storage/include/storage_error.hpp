#pragma once

#include <stdexcept>
#include <string>

namespace storage {

enum class ErrorCode {
  NotFound,
  DecodeError,
  IoError,
  InvalidPath,
};

class StorageError : public std::runtime_error {
public:
  explicit StorageError(ErrorCode code, const std::string &message = "")
      : std::runtime_error(message.empty() ? codeString(code) : message), code_(code) {}

  ErrorCode code() const { return code_; }

private:
  static const char *codeString(ErrorCode c) {
    switch (c) {
    case ErrorCode::NotFound:
      return "Not found";
    case ErrorCode::DecodeError:
      return "Decode error";
    case ErrorCode::IoError:
      return "I/O error";
    case ErrorCode::InvalidPath:
      return "Invalid path";
    default:
      return "Unknown storage error";
    }
  }
  ErrorCode code_;
};

} // namespace storage
