#ifndef METRIC_H
#define METRIC_H

#include <algorithm>

namespace call_center::core::qs::metrics {

/**
 * @brief Класс для хранения информации по конкретному показателю.
 *
 * Он сохраняет минимальное, максимальное и среднее значение данного показателя.
 * @tparam T значение показателя
 * @tparam AvgT среднее значение показателя
 */
template <typename T, typename AvgT = T>
class Metric {
 public:
  /**
   * @param default_value значение показателя по умолчанию
   */
  explicit Metric(T default_value);

  /**
   * @brief Добавить новое значение показателя.
   */
  void AddValue(T value);
  /**
   * @brief Минимальное значение показателя.
   */
  [[nodiscard]] T GetMin() const;
  /**
   * @brief Максимальное значение показателя.
   */
  [[nodiscard]] T GetMax() const;
  /**
   * @brief Среднее значение показателя.
   */
  [[nodiscard]] AvgT GetAvg() const;
  /**
   * @brief Количество зафиксированных значений.
   */
  [[nodiscard]] size_t GetCount() const;

 private:
  T min_;
  T max_;
  AvgT avg_;
  size_t count_ = 0;
};

template <typename T, typename AvgT>
Metric<T, AvgT>::Metric(T default_value)
    : min_(default_value), max_(default_value), avg_(default_value) {
}

template <typename T, typename AvgT>
void Metric<T, AvgT>::AddValue(T value) {
  ++count_;
  min_ = std::min(value, min_);
  max_ = std::max(value, max_);
  if (value > avg_) {
    avg_ += AvgT(value - avg_) / count_;
  } else {
    avg_ -= AvgT(avg_ - value) / count_;
  }
}

template <typename T, typename AvgT>
T Metric<T, AvgT>::GetMin() const {
  return min_;
}

template <typename T, typename AvgT>
T Metric<T, AvgT>::GetMax() const {
  return max_;
}

template <typename T, typename AvgT>
AvgT Metric<T, AvgT>::GetAvg() const {
  return avg_;
}

template <typename T, typename AvgT>
size_t Metric<T, AvgT>::GetCount() const {
  return count_;
}

}  // namespace call_center::core::qs::metrics

#endif  // METRIC_H
