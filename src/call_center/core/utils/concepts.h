#ifndef CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_CONCEPTS_H_
#define CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_CONCEPTS_H_

#include <type_traits>

namespace call_center::core::utils::concepts {

template<typename T>
concept NoThrowCopyConstructor = std::is_nothrow_copy_constructible_v<T>;

template<typename T>
concept NoThrowMoveConstructor = std::is_nothrow_move_constructible_v<T>;

}

#endif //CALL_CENTER_SRC_CALL_CENTER_CORE_UTILS_CONCEPTS_H_
