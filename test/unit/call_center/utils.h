#ifndef CALL_CENTER_TEST_UNIT_CALL_CENTER_UTILS_H_
#define CALL_CENTER_TEST_UNIT_CALL_CENTER_UTILS_H_

#include <future>
#include <string>

namespace call_center::test {

void CreateDirForLogs(const std::string &test_group);
void CreateDirForConfigs(const std::string &test_group);

template <typename T>
auto CapturePromise(std::promise<T> &&promise) {
  return std::make_shared<std::promise<T>>(std::move(promise));
}

}  // namespace call_center::test

#endif  // CALL_CENTER_TEST_UNIT_CALL_CENTER_UTILS_H_
