#ifndef SERVER_H
#define SERVER_H

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>

namespace call_center::qs {

class Server {
 public:
  using Id = boost::uuids::uuid;

  explicit Server(Id id = boost::uuids::random_generator_mt19937()());
  virtual ~Server() = default;

  [[nodiscard]] virtual bool IsFree() const = 0;
  [[nodiscard]] virtual bool IsBusy() const = 0;
  [[nodiscard]] virtual bool operator==(const Server& other) const;
  [[nodiscard]] virtual size_t hash() const;

  [[nodiscard]] Id GetId() const;

 protected:
  const Id id_;
};

}  // namespace call_center::qs

#endif  // SERVER_H
