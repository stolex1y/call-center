#ifndef CALL_CENTER_SRC_CALL_CENTER_LOG_ATTRS_H_
#define CALL_CENTER_SRC_CALL_CENTER_LOG_ATTRS_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/uuid/uuid.hpp>

#include "severity_level.h"

/// Основные атрибуты для логирования @link attrs.h @endlink.
namespace call_center::log::attrs {

BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", boost::uuids::uuid)
BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", uint)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", SeverityLevel)
BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr, "Tag", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(thread, "ThreadID", boost::log::thread_id)

}  // namespace call_center::log::attrs

#endif  // CALL_CENTER_SRC_CALL_CENTER_LOG_ATTRS_H_
