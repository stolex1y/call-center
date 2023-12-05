#ifndef CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_UUIDS_H_
#define CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_UUIDS_H_

#include <boost/uuid/uuid_io.hpp>

/// Вспомогательные классы для работы с boost::uuids::uuid.
namespace call_center::core::utils::uuids {

/**
 * @brief Вывести boost::uuids::uuid в std::ostream.
 */
std::ostream &operator<<(std::ostream &out, boost::uuids::uuid id);

}  // namespace call_center::core::utils::uuids

#endif  // CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_UUIDS_H_
