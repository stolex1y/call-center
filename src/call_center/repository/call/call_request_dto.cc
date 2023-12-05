#include "call_request_dto.h"

namespace call_center::repository {

CallRequestDto tag_invoke(const json::value_to_tag<CallRequestDto> &, const json::value &json) {
  const json::object &json_obj = json.as_object();

  return {.phone = std::string(json_obj.at("phone").as_string())};
}

}  // namespace call_center::repository
