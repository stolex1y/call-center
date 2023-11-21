#ifndef CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_UUIDS_H_
#define CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_UUIDS_H_

#include <boost/uuid/uuid_io.hpp>

namespace call_center::core::utils {

std::ostream &operator<<(std::ostream &out, boost::uuids::uuid id);

}  // namespace call_center::core::utils

#endif  // CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_UUIDS_H_
