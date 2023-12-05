#ifndef CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_H_
#define CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_H_

#include <boost/asio.hpp>
#include <chrono>
#include <functional>

/// Планирование и выполнение задач.
namespace call_center::core::tasks {

/**
 * @brief Базовый класс для менеджера задач.
 *
 * Помимо выполнения пользовательских задач, содержит boost::asio::io_context для задач
 * ввода-вывода.
 */
class TaskManager {
 public:
  using Task = void();

  TaskManager() = default;
  TaskManager(const TaskManager &other) = delete;
  TaskManager &operator=(const TaskManager &other) = delete;
  virtual ~TaskManager() = default;

  /**
   * @brief Запустить обработку задач.
   */
  virtual void Start() = 0;
  /**
   * @brief Завершить обработку задач. После вызова данной функции нельзя повторно запустить
   * @link TaskManager @endlink.
   */
  virtual void Stop() = 0;
  /**
   * @brief Ожидать остановки @link TaskManager @endlink.
   */
  virtual void Join() = 0;
  virtual boost::asio::io_context &IoContext() = 0;
  /**
   * @brief Поставить задачу на выполнение.
   */
  virtual void PostTask(std::function<Task> task) = 0;

  /**
   * @brief В указанное время (time_point) выполнить задачу.
   * @param time_point время, в которое нужно выполнить задачу
   */
  template <typename TimePoint>
  void PostTaskAt(const TimePoint &time_point, const std::function<Task> &task);

  /**
   * @brief Через указанную задержку (delay) выполнить задачу.
   * @param delay задержка, через которую нужно выполнить задачу
   */
  template <typename Duration>
  void PostTaskDelayed(const Duration &delay, const std::function<Task> &task);

 protected:
  using Clock_t = std::chrono::utc_clock;
  using Duration_t = std::chrono::milliseconds;
  using TimePoint_t = std::chrono::time_point<Clock_t, Duration_t>;

  /**
   * @brief Конкретная реализация метода @link PostTaskDelayed @endlink, но с определенными
   * временными единицами.
   */
  virtual void PostTaskDelayedImpl(Duration_t delay, std::function<Task> task) = 0;
  /**
   * @brief Конкретная реализация метода @link PostTaskAt @endlink, но с определенными
   * временными единицами.
   */
  virtual void PostTaskAtImpl(TimePoint_t time_point, std::function<Task> task) = 0;
};

template <typename Duration>
void TaskManager::PostTaskDelayed(const Duration &delay, const std::function<Task> &task) {
  PostTaskDelayedImpl(std::chrono::duration_cast<Duration_t>(delay), task);
}

template <typename TimePoint>
void TaskManager::PostTaskAt(const TimePoint &time_point, const std::function<Task> &task) {
  PostTaskAtImpl(
      std::chrono::time_point_cast<Duration_t, Clock_t, typename TimePoint::duration>(time_point),
      task
  );
}

}  // namespace call_center::core::tasks

#endif  // CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_H_
