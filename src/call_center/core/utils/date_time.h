#ifndef CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_DATE_TIME_H_
#define CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_DATE_TIME_H_

#include <boost/date_time.hpp>
#include <chrono>

/// Вспомогательные классы для работы с датой и временем.
namespace call_center::core::utils::date_time {

using namespace std::chrono_literals;

/**
 * @brief Извлечь время из временной точки.
 */
template <typename Duration, typename TimePoint>
std::chrono::hh_mm_ss<Duration> ExtractTime(const TimePoint &time_point) {
  auto dp = std::chrono::floor<Duration>(time_point);
  return std::chrono::hh_mm_ss{time_point - dp};
}

/**
 * @brief Извлечь дату из временной точки.
 */
template <typename TimePoint>
std::chrono::year_month_day ExtractDate(const TimePoint &time_point) {
  return std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(time_point)};
}

/**
 * @brief Преобразовать duration из std в boost.
 */
template <typename BoostDuration, typename StdDuration>
BoostDuration CastToBoost(const StdDuration &duration) {
  auto ns =
      boost::chrono::nanoseconds{std::chrono::duration_cast<std::chrono::nanoseconds>(duration)};
  return boost::chrono::duration_cast<BoostDuration>(ns);
}

/**
 * @brief Преобразовать time_point из std в boost.
 */
template <typename BoostTimePoint, typename StdTimePoint>
BoostTimePoint CastToBoostTimePoint(const StdTimePoint &time_point) {
  using BoostClock = typename BoostTimePoint::clock;
  using BoostDuration = typename BoostTimePoint::duration;

  auto duration_ns = CastToBoost<boost::chrono::nanoseconds>(
      std::chrono::time_point_cast<std::chrono::nanoseconds>(time_point).time_since_epoch()
  );
  return boost::chrono::time_point_cast<BoostDuration, BoostClock>(
      boost::chrono::time_point<BoostClock, boost::chrono::microseconds>{duration_ns}
  );
}

}  // namespace call_center::core::utils::date_time

#endif  // CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_DATE_TIME_H_
