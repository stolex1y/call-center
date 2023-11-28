#ifndef REQUEST_H
#define REQUEST_H

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <optional>

namespace call_center::qs {

class Request {
 public:
  using Id = boost::uuids::uuid;
  using Clock = std::chrono::utc_clock;
  using Duration = std::chrono::milliseconds;
  using WaitingDuration = std::chrono::seconds;
  using TimePoint = std::chrono::time_point<Clock, Duration>;

  explicit Request(Id id = boost::uuids::random_generator_mt19937()());
  virtual ~Request() = default;

  [[nodiscard]] virtual std::optional<Duration> GetServiceTime() const = 0;
  [[nodiscard]] virtual std::optional<Duration> GetTotalTime() const = 0;
  [[nodiscard]] virtual std::optional<TimePoint> GetArrivalTime() const = 0;
  [[nodiscard]] virtual std::optional<TimePoint> GetServiceCompleteTime() const = 0;
  [[nodiscard]] virtual std::optional<TimePoint> GetServiceStartTime() const = 0;
  [[nodiscard]] virtual std::optional<Duration> GetWaitTime() const = 0;
  [[nodiscard]] virtual bool WasFinished() const = 0;
  [[nodiscard]] virtual bool WasServiced() const = 0;
  [[nodiscard]] virtual bool WasArrived() const = 0;
  [[nodiscard]] virtual bool IsTimeout() const = 0;

  [[nodiscard]] virtual size_t hash() const;

  [[nodiscard]] bool operator==(const Request &other) const;
  [[nodiscard]] Id GetId() const;

 protected:
  const Id id_;
};

}  // namespace call_center::qs

template <>
struct std::hash<call_center::qs::Request> {
  size_t operator()(const call_center::qs::Request &request) const noexcept {
    return request.hash();
  }
};

#endif  // REQUEST_H
