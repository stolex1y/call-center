#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_REQUEST_DTO_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_REQUEST_DTO_H_

#include <boost/json.hpp>
#include <string>

namespace call_center::repository {
namespace json = boost::json;

struct CallRequestDto {
  std::string phone;

  friend CallRequestDto tag_invoke(
      const json::value_to_tag<CallRequestDto> &, const json::value &json
  );
};

}  // namespace call_center::repository

#endif  // CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_REQUEST_DTO_H_
