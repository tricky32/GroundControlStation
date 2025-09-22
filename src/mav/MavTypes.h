#pragma once
#include <common/mavlink.h>

// Single source of truth for the MAVLink message type alias used in the app.
using MavMsg = mavlink_message_t;

// Make sure Qt knows how to pass this type through signals/slots (queued connections).
#include <QMetaType>
Q_DECLARE_METATYPE(mavlink_message_t)
