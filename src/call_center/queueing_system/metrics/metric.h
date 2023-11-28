#ifndef METRIC_H
#define METRIC_H
#include <algorithm>

namespace call_center::qs::metrics {

template <typename T>
class Metric {
 public:
  explicit Metric(T default_value);

  void AddValue(T value);
  [[nodiscard]] T GetMin() const;
  [[nodiscard]] T GetMax() const;
  [[nodiscard]] T GetAvg() const;
  [[nodiscard]] size_t GetCount() const;

 private:
  T min_;
  T max_;
  T avg_;
  size_t count_ = 0;
};

template <typename T>
Metric<T>::Metric(T default_value) : min_(default_value), max_(default_value), avg_(default_value) {
}

template <typename T>
void Metric<T>::AddValue(T value) {
  ++count_;
  min_ = std::min(value, min_);
  max_ = std::max(value, max_);
  avg_ += (value - T(1)) / count_;
}

template <typename T>
T Metric<T>::GetMin() const {
  return min_;
}

template <typename T>
T Metric<T>::GetMax() const {
  return max_;
}

template <typename T>
T Metric<T>::GetAvg() const {
  return avg_;
}

template <typename T>
size_t Metric<T>::GetCount() const {
  return count_;
}

}  // namespace call_center::qs::metrics

#endif  // METRIC_H
