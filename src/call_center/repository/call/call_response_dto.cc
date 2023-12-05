#include "call_response_dto.h"

namespace call_center::repository {

void tag_invoke(
    const json::value_from_tag &, json::value &json, const CallResponseDto &call_response
) {
  json = {{"call_status", call_response.call_status}};
}

CallResponseDto::CallResponseDto(const CallStatus status) : call_status(to_string(status)) {
}

}  // namespace call_center::repository
