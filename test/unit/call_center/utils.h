#ifndef CALL_CENTER_TEST_UNIT_CALL_CENTER_UTILS_H_
#define CALL_CENTER_TEST_UNIT_CALL_CENTER_UTILS_H_

#include <string>

namespace call_center::test {

void CreateDirForLogs(const std::string &test_group);
void CreateDirForConfigs(const std::string &test_group);

}  // namespace call_center::test

#endif  // CALL_CENTER_TEST_UNIT_CALL_CENTER_UTILS_H_
