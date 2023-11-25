#include "utils.h"

#include <filesystem>

namespace call_center::test {

void CreateDirForLogs(const std::string &test_group) {
  std::filesystem::create_directories(test_group + "/logs");
}

void CreateDirForConfigs(const std::string &test_group) {
  std::filesystem::create_directories(test_group + "/configs");
}

}  // namespace call_center::test
