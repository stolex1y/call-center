#include "request.h"

#include <boost/container_hash/hash.hpp>

namespace call_center::core::qs {

Request::Request(const Id id) : id_(id) {
}

bool Request::operator==(const Request& other) const {
  return id_ == other.id_;
}

size_t Request::hash() const {
  return boost::hash<Id>()(id_);
}

Request::Id Request::GetId() const {
  return id_;
}

}  // namespace call_center::core::qs
