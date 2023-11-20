#include "call_status.h"

#include <stdexcept>

namespace call_center {

std::string to_string(CallStatus status) {
  switch (status) {
    case CallStatus::kOk: {
      return "ok";
    }
    case CallStatus::kOverload: {
      return "overload";
    }
    case CallStatus::kTimeout: {
      return "timout";
    }
    case CallStatus::kAlreadyInQueue: {
      return "already in queue";
    }
    default: {
      throw std::runtime_error("Unhandled enum constant");
    }
  }
}

std::ostream &operator<<(std::ostream &out, CallStatus status) {
  return out << to_string(status);
}

}  // namespace call_center
