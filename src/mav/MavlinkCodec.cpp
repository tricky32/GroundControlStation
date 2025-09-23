#include "MavlinkCodec.h"

void MavlinkCodec::feed(const QByteArray& bytes, const Endpoint& ep){
    mavlink_message_t msg;
    mavlink_status_t status{};
    for(unsigned char c: bytes){
        if(mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)){
            emit messageReceived(msg, ep);
        }
    }
}

QByteArray MavlinkCodec::packMsg(const mavlink_message_t& msg){
    QByteArray buf; buf.resize(MAVLINK_MAX_PACKET_LEN);
    int len = mavlink_msg_to_send_buffer(reinterpret_cast<uint8_t*>(buf.data()), &msg);
    buf.resize(len);
    return buf;
}
