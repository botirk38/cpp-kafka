#include "internal/storage_service_impl.hpp"
#include "storage_service.hpp"

namespace storage {

std::unique_ptr<IStorageService> createStorageService(std::string base_path) {
  return std::make_unique<internal::StorageServiceImpl>(std::move(base_path));
}

} // namespace storage
