#include "uuids.h"

namespace call_center::core::utils::uuids {

std::ostream &operator<<(std::ostream &out, boost::uuids::uuid id) {
  return out << "'" << to_string(id) << "'";
}

}  // namespace call_center::core::utils::uuids