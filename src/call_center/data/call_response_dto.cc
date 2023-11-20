#include "call_response_dto.h"

namespace call_center::data {

void tag_invoke(
    const json::value_from_tag &,
    json::value &json,
    const CallResponseDto &call_response
) {
  json = {{"call_status", call_response.call_status}};
}

}  // namespace call_center::data
