#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_RESPONSE_DTO_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_RESPONSE_DTO_H_

#include <boost/json.hpp>
#include <string>

#include "call_status.h"

namespace call_center::repository {

namespace json = boost::json;

struct CallResponseDto {
  std::string call_status;

  explicit CallResponseDto(CallStatus status);

  /**
   * @brief Преоразование из объекта в json.
   */
  friend void tag_invoke(
      const json::value_from_tag &, json::value &json, const CallResponseDto &call_response
  );
};

}  // namespace call_center::repository

#endif  // CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_RESPONSE_DTO_H_
