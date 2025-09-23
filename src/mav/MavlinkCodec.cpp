#include "MavlinkCodec.h"

void MavlinkCodec::feed(const QByteArray& bytes, const Endpoint& ep) {
    mavlink_message_t msg{};
    mavlink_status_t  status{};

    const uint8_t* data = reinterpret_cast<const uint8_t*>(bytes.constData());
    for (int i = 0; i < bytes.size(); ++i) {
        if (mavlink_parse_char(MAVLINK_COMM_0, data[i], &msg, &status)) {
            emit messageReceived(msg, ep);
        }
    }
}
