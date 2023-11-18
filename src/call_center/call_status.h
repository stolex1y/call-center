#ifndef CALL_CENTER_SRC_CALL_CENTER_CALL_STATUS_H_
#define CALL_CENTER_SRC_CALL_CENTER_CALL_STATUS_H_

#include <string>
#include <ostream>

namespace call_center {

enum class CallStatus {
  kOk,
  kOverload,
  kTimeout,
  kAlreadyInQueue
};

std::string to_string(CallStatus status);
std::ostream &operator<<(std::ostream &out, CallStatus status);

} // call_center

#endif //CALL_CENTER_SRC_CALL_CENTER_CALL_STATUS_H_
