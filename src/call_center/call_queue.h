#ifndef CALL_CENTER_SRC_CALL_CENTER_CALL_QUEUE_H_
#define CALL_CENTER_SRC_CALL_CENTER_CALL_QUEUE_H_

#include <functional>
#include <set>
#include <unordered_set>

#include "call_detailed_record.h"
#include "configuration/configuration.h"

namespace call_center {

/**
 * @brief Очередь звонков.
 *
 * Помимо базового интерфейса очереди, поддерживает дополнительную функциональность.
 * Например, звонки в очереди должны быть уникальны, причем как находящиеся непосредственно в
 * очереди, но и на обслуживании.
 * Кроме того, необходимо иметь возможность просматривать звонки в очереди в порядке истечения
 * времени ожидания, а также иметь возможность удалить звонок из середины очереди, например, если
 * его время ожидания вышло.
 */
class CallQueue {
 public:
  using CallPtr = std::shared_ptr<CallDetailedRecord>;

  /**
   * @brief Результат добавления вызова в очередь.
   */
  enum class PushResult {
    kOk,              ///< Успешное добавление вызова.
    kAlreadyInQueue,  ///< Запрос уже находится в очереди.
    kOverload  ///< Количество запросов в очереди достигло максимально допустимого значения.
  };

  /// Ключ в конфигурации, соответствующий значению емкости очереди.
  static constexpr auto kCapacityKey = "queue_capacity";

  CallQueue(
      std::shared_ptr<config::Configuration> configuration,
      const log::LoggerProvider &logger_provider
  );
  CallQueue(const CallQueue &other) = delete;
  CallQueue &operator=(const CallQueue &other) = delete;

  /**
   * @brief Извлечь следующий по порядку запрос из очереди.
   * @return nullptr - если очередь пуста.
   */
  CallPtr PopFromQueue();
  /**
   * @brief Добавить запрос в очередь.
   */
  PushResult PushToQueue(const CallPtr &call);
  /**
   * @brief Содержатся ли в очереди запросы для обслуживания.
   */
  [[nodiscard]] bool QueueIsEmpty() const;
  /**
   * @brief Извлечь из очереди запрос, время ожидания которого истекло.
   * @return nullptr - если такого запроса нет.
   */
  CallPtr EraseTimeoutCallFromQueue();
  /**
   * @brief Получить, не удаляя, из очереди запрос наиболее близкий к таймауту.
   * @return nullptr - если очередь пуста.
   */
  CallPtr GetMinTimeoutCallInQueue() const;
  /**
   * @brief Удалить запрос из множества обслуживаемых запросов.
   */
  void EraseFromProcessing(const CallPtr &call);
  /**
   * @brief Добавить запрос в множества обслуживаемых запросов.
   * @return false - если такой запрос уже был добавлен, иначе - true.
   */
  bool InsertToProcessing(const CallPtr &call);
  /**
   * @brief Содержится ли в очереди либо на выполнении указанный запрос.
   */
  [[nodiscard]] bool Contains(const CallPtr &call) const;
  /**
   * @brief Количество запросов, находящихся в очереди на обслуживание.
   */
  [[nodiscard]] size_t GetSize() const;
  /**
   * @brief Максимальное количество запросов, которые могут быть в очереди на обслуживание.
   */
  [[nodiscard]] size_t GetCapacity() const;

 private:
  struct CallEquals {
    bool operator()(const CallPtr &first, const CallPtr &second) const;
  };

  struct CallHash {
    size_t operator()(const CallPtr &call) const;
  };

  /**
   * @brief Компаратор для упорядочивания запросов в порядке поступления в систему.
   */
  struct ReceiptOrder {
    bool operator()(const CallPtr &first, const CallPtr &second) const;
  };

  /**
   * @brief Компаратор для упорядочивания запросов в порядке истечения времени ожидания.
   */
  struct TimeoutPointOrder {
    bool operator()(const CallPtr &first, const CallPtr &second) const;
  };

  static constexpr size_t kDefaultCapacity_ = 10;

  std::unordered_set<CallPtr, CallHash, CallEquals> in_processing_;
  std::multiset<CallPtr, TimeoutPointOrder> in_timout_point_order_;
  std::multiset<CallPtr, ReceiptOrder> in_receipt_order_;
  mutable std::shared_mutex queue_mutex_;
  std::unique_ptr<log::Logger> logger_;
  const std::shared_ptr<config::Configuration> configuration_;
  size_t capacity_ = kDefaultCapacity_;

  /**
   * @brief Обновить емкость очереди, согласно значению из конфигурации.
   */
  void UpdateCapacity();
  /**
   * @brief Удалить запрос из очереди.
   */
  void EraseFromQueue(const CallPtr &call);
  /**
   * @brief Добавить запрос в очередь.
   */
  void InsertToQueue(const CallPtr &call);

  /**
   * @brief Удалить только один запрос из мультисета.
   *
   * Например, может сложиться ситуация, когда окончание времени ожидания будет в один и тот же
   * момент, поэтому с этой точки зрения запросы будут равны, однако необходимо удалить только
   * переданный в аргументах запрос.
   */
  template <typename Cmp>
  static void EraseCallFromMultiset(std::multiset<CallPtr, Cmp> &multiset, const CallPtr &call);

  /**
   * @brief Содержит ли мультисет указанный запрос вне зависимости от компаратора.
   */
  template <typename Cmp>
  static bool MultisetContainsCall(
      const std::multiset<CallPtr, Cmp> &multiset, const CallPtr &call
  );
};

template <typename Cmp>
void CallQueue::EraseCallFromMultiset(std::multiset<CallPtr, Cmp> &multiset, const CallPtr &call) {
  constexpr CallEquals equals;
  for (auto [begin, end] = multiset.equal_range(call); begin != end; ++begin) {
    if (equals(*begin, call)) {
      multiset.erase(begin);
      break;
    }
  }
}

template <typename Cmp>
bool CallQueue::MultisetContainsCall(
    const std::multiset<CallPtr, Cmp> &multiset, const CallPtr &call
) {
  constexpr CallEquals equals;
  for (auto [begin, end] = multiset.equal_range(call); begin != end; ++begin) {
    if (equals(*begin, call)) {
      return true;
    }
  }
  return false;
}

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CALL_QUEUE_H_
