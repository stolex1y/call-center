#ifndef CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_DATE_TIME_H_
#define CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_DATE_TIME_H_

#include <boost/date_time.hpp>
#include <chrono>

namespace call_center::core::utils::date_time {

using namespace std::chrono_literals;

template <typename Duration, typename TimePoint>
std::chrono::hh_mm_ss<Duration> ExtractTime(const TimePoint &time_point) {
  auto dp = std::chrono::floor<Duration>(time_point);
  return std::chrono::hh_mm_ss{time_point - dp};
}

template <typename Duration>
std::chrono::year_month_day ExtractDate(
    const std::chrono::sys_time<Duration> &time_point
) {
  return std::chrono::year_month_day{
      std::chrono::floor<std::chrono::days>(time_point)};
}

template <typename Duration>
std::chrono::year_month_day ExtractDate(
    const std::chrono::utc_time<Duration> &time_point
) {
  return std::chrono::year_month_day{
      std::chrono::floor<std::chrono::days>(time_point)};
}

template <typename BoostDuration, typename StdDuration>
BoostDuration CastToBoost(const StdDuration &duration) {
  auto ms = boost::chrono::microseconds{
      std::chrono::duration_cast<std::chrono::microseconds>(duration)};
  return boost::chrono::duration_cast<BoostDuration>(ms);
}

template <typename BoostTimePoint, typename StdTimePoint>
BoostTimePoint CastToBoostTimePoint(const StdTimePoint &time_point) {
  using BoostClock = BoostTimePoint::clock;
  using BoostDuration = BoostTimePoint::duration;

  auto duration_us = CastToBoost<boost::chrono::microseconds>(
      std::chrono::time_point_cast<std::chrono::microseconds>(time_point)
          .time_since_epoch()
  );
  return boost::chrono::time_point_cast<BoostDuration, BoostClock>(
      boost::chrono::time_point<BoostClock, boost::chrono::microseconds>{
          duration_us}
  );
}

}  // namespace call_center::core::utils::date_time

#endif  // CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_DATE_TIME_H_
