#include "MavCommandSender.h"
#include "MavRouter.h"
#include <QDebug>
#include <QTimer>
#include <chrono>
using namespace std::chrono_literals;

MavCommandSender::MavCommandSender(MavRouter* router, QObject* parent)
    : QObject(parent), router_(router) {
    connect(router_, &MavRouter::mavMessageReceived,
            this, &MavCommandSender::onMavMessage);
}

void MavCommandSender::sendCommandLong_(int sysid, int command,
                                        float p1, float p2, float p3, float p4,
                                        float p5, float p6, float p7,
                                        int targetComp)
{
    mavlink_command_long_t cmd{};
    cmd.target_system    = static_cast<uint8_t>(sysid);
    cmd.target_component = static_cast<uint8_t>(targetComp);
    cmd.command          = static_cast<uint16_t>(command);
    cmd.confirmation     = 0;
    cmd.param1 = p1; cmd.param2 = p2; cmd.param3 = p3; cmd.param4 = p4;
    cmd.param5 = p5; cmd.param6 = p6; cmd.param7 = p7;

    mavlink_message_t msg;
    mavlink_msg_command_long_encode(255, MAV_COMP_ID_MISSIONPLANNER, &msg, &cmd);

    QByteArray bytes;
    bytes.resize(MAVLINK_MAX_PACKET_LEN);
    const int len = mavlink_msg_to_send_buffer(reinterpret_cast<uint8_t*>(bytes.data()), &msg);
    bytes.resize(len);

    const quint64 key = (static_cast<quint64>(sysid) << 32) | static_cast<quint64>(command);
    pending_.insert(key, PendingCmd{bytes, sysid, command, 0});

    emit commandSent(sysid, command);
    if (router_) router_->onCmdBytes(bytes, sysid);
    scheduleRetry_(key);
}

void MavCommandSender::scheduleRetry_(quint64 key) {
    QTimer::singleShot(900ms, this, [this, key]() {
        auto it = pending_.find(key);
        if (it == pending_.end()) return;
        if (it->retries >= 3) {
            qWarning() << "MAVCmd" << it->command << "sys" << it->sysid
                       << "FAILED (no ACK after retries)";
            pending_.erase(it);
            return;
        }
        if (router_) router_->onCmdBytes(it->bytes, it->sysid);
        it->retries++;
        scheduleRetry_(key);
    });
}

void MavCommandSender::onMavMessage(const mavlink_message_t& msg) {
    if (msg.msgid != MAVLINK_MSG_ID_COMMAND_ACK) return;
    mavlink_command_ack_t ack{};
    mavlink_msg_command_ack_decode(&msg, &ack);

    const quint64 key = (static_cast<quint64>(msg.sysid) << 32) | static_cast<quint64>(ack.command);
    auto it = pending_.find(key);
    if (it == pending_.end()) return;

    qDebug() << "COMMAND_ACK cmd" << ack.command
             << "sys" << msg.sysid
             << "result" << ack.result;

    pending_.erase(it);
}

void MavCommandSender::arm(int sysid, bool force) {
    sendCommandLong_(sysid, MAV_CMD_COMPONENT_ARM_DISARM,
                     1.0f, force ? 21196.0f : 0.0f,
                     0,0,0,0,0,
                     MAV_COMP_ID_AUTOPILOT1);
}

void MavCommandSender::disarm(int sysid) {
    sendCommandLong_(sysid, MAV_CMD_COMPONENT_ARM_DISARM,
                     0.0f, 0.0f,
                     0,0,0,0,0,
                     MAV_COMP_ID_AUTOPILOT1);
}

void MavCommandSender::takeoff(int sysid, float alt) {
    sendCommandLong_(sysid, MAV_CMD_NAV_TAKEOFF,
                     0,0,0,0,
                     0,0,alt,
                     MAV_COMP_ID_AUTOPILOT1);
}

void MavCommandSender::land(int sysid) {
    sendCommandLong_(sysid, MAV_CMD_NAV_LAND,
                     0,0,0,0,
                     0,0,0,
                     MAV_COMP_ID_AUTOPILOT1);
}

void MavCommandSender::changeAlt(int sysid, float alt) {
    sendCommandLong_(sysid, MAV_CMD_DO_CHANGE_ALTITUDE,
                     alt, 0,0,0,0,0,0,
                     MAV_COMP_ID_AUTOPILOT1);
}

void MavCommandSender::setMode(int sysid, uint8_t baseMode, uint32_t customMode) {
    sendCommandLong_(sysid, MAV_CMD_DO_SET_MODE,
                     static_cast<float>(baseMode),
                     static_cast<float>(customMode),
                     0,0,0,0,0,
                     MAV_COMP_ID_AUTOPILOT1);
}

void MavCommandSender::rtl(int sysid) {
    setMode(sysid, MAV_MODE_FLAG_CUSTOM_MODE_ENABLED, 11); // AC RTL
    sendCommandLong_(sysid, MAV_CMD_NAV_RETURN_TO_LAUNCH,
                     0,0,0,0, 0,0,0,
                     MAV_COMP_ID_AUTOPILOT1);
}

void MavCommandSender::gotoLatLonAlt(int sysid, double lat, double lon, double alt) {
    setMode(sysid, MAV_MODE_FLAG_CUSTOM_MODE_ENABLED, 4); // Guided
    sendCommandLong_(sysid, MAV_CMD_NAV_WAYPOINT,
                     0,0,0,0,
                     static_cast<float>(lat),
                     static_cast<float>(lon),
                     static_cast<float>(alt),
                     MAV_COMP_ID_AUTOPILOT1);
}
