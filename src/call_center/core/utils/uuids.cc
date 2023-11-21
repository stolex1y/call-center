#include "uuids.h"

namespace call_center::core::utils {

std::ostream &operator<<(std::ostream &out, boost::uuids::uuid id) {
  return out << "'" << boost::uuids::to_string(id) << "'";
}

}  // namespace call_center::core::utils