#ifndef CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_H_
#define CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_H_

#include <string>
#include <optional>

#include <boost/core/ignore_unused.hpp>

namespace call_center {

class Configuration {
 public:
  template<typename T>
  std::optional<T> GetProperty(const std::string &key) const;

 private:
};

template<typename T>
std::optional<T> Configuration::GetProperty(const std::string &key) const {
  boost::ignore_unused(key);
  return std::nullopt;
}

} // call_center

#endif //CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_H_
