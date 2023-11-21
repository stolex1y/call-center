#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_RESPONSE_DTO_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_RESPONSE_DTO_H_

#include <boost/json.hpp>
#include <string>

namespace call_center::data {

namespace json = boost::json;

struct CallResponseDto {
  std::string call_status;

  friend void tag_invoke(
      const json::value_from_tag &, json::value &json, const CallResponseDto &call_response
  );
};

}  // namespace call_center::data

#endif  // CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_RESPONSE_DTO_H_
