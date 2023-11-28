#include "server.h"

#include <boost/container_hash/hash.hpp>

namespace call_center::qs {

Server::Server(Id id) : id_(id) {
}

bool Server::operator==(const Server& other) const {
  return id_ == other.id_;
}

size_t Server::hash() const {
  return boost::hash<Id>()(id_);
}

Server::Id Server::GetId() const {
  return id_;
}

}  // namespace call_center::qs
