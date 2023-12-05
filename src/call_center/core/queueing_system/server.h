#ifndef SERVER_H
#define SERVER_H

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>

namespace call_center::core::qs {

/**
 * @brief Базовый класс обслуживающего прибора в СМО.
 *
 * Имеет состояние, определяющее занятость прибора обслуживанием запроса.
 */
class Server {
 public:
  using Id = boost::uuids::uuid;

  explicit Server(Id id = boost::uuids::random_generator_mt19937()());
  virtual ~Server() = default;

  /**
   * @brief Свободен ли данный прибор.
   */
  [[nodiscard]] virtual bool IsFree() const = 0;
  /**
   * @brief Занят ли обслуживание данный прибор.
   */
  [[nodiscard]] virtual bool IsBusy() const = 0;
  [[nodiscard]] virtual bool operator==(const Server& other) const;
  [[nodiscard]] virtual size_t hash() const;

  /**
   * @brief Уникальный идентификатор прибора.
   */
  [[nodiscard]] Id GetId() const;

 protected:
  const Id id_;
};

}  // namespace call_center::core::qs

#endif  // SERVER_H
