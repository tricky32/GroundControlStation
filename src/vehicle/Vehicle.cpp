#include "Vehicle.h"
#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtMath>
#include <QtPositioning/QGeoCoordinate>


static inline uint32_t nowMs32() {
    return static_cast<uint32_t>(QDateTime::currentMSecsSinceEpoch() & 0xFFFFFFFFu);
}
static inline qint64 nowMs64() {
    return QDateTime::currentMSecsSinceEpoch();
}

Vehicle::Vehicle(int sysId, const Endpoint& ep, QObject* parent)
    : QObject(parent), m_sysId(sysId), m_ep(ep)
{
    m_lastHeartbeat.start();

    // command retry timer
    m_cmdTimer.setInterval(100);
    connect(&m_cmdTimer, &QTimer::timeout, this, [this](){
        if (m_pending.isEmpty()) { m_cmdTimer.stop(); return; }
        const qint64 now = nowMs64();
        for (int i = m_pending.size()-1; i >= 0; --i) {
            auto &pc = m_pending[i];
            if (now >= pc.dueAtMs) {
                if (pc.retries <= 0) {
                    emit commandFailed(pc.cmd, MAV_RESULT_FAILED);
                    m_pending.removeAt(i);
                    continue;
                }
                // resend
                emit sendBytes(m_ep, pc.bytes);
                pc.retries--;
                pc.dueAtMs = now + pc.timeoutMs;
            }
        }
        if (m_pending.isEmpty()) m_cmdTimer.stop();
    });
}

void Vehicle::sendMessage(const mavlink_message_t& m) {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    const uint16_t len = mavlink_msg_to_send_buffer(buf, &m);
    emit sendBytes(m_ep, QByteArray(reinterpret_cast<const char*>(buf), len));
}

void Vehicle::handleMsg(const mavlink_message_t& msg) {
    m_lastHeartbeat.restart();

    switch (msg.msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT: {
        mavlink_heartbeat_t hb; mavlink_msg_heartbeat_decode(&msg, &hb);
        m_mode = hb.custom_mode;
        m_armed = (hb.base_mode & MAV_MODE_FLAG_SAFETY_ARMED);
        updateStatus(); emit telemetryChanged(); break;
    }
    case MAVLINK_MSG_ID_EXTENDED_SYS_STATE: {
        mavlink_extended_sys_state_t s; mavlink_msg_extended_sys_state_decode(&msg, &s);
        m_inAir = (s.landed_state == MAV_LANDED_STATE_IN_AIR || s.landed_state == MAV_LANDED_STATE_TAKEOFF);
        updateStatus(); emit telemetryChanged(); break;
    }
    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: {
        mavlink_global_position_int_t gp; mavlink_msg_global_position_int_decode(&msg, &gp);
        m_position.latitude  = gp.lat / 1e7;
        m_position.longitude = gp.lon / 1e7;
        m_position.altitude  = gp.relative_alt / 1000.0; // m (relative)
        m_position.isValid   = true;
        m_altAMSL            = gp.alt / 1000.0;          // m (AMSL)
        m_heading            = gp.hdg / 100.0;
        emit telemetryChanged();

        // clear goto when reached (≈ within 2 m horizontal and 1 m vertical)
        if (m_goto.isValid) {
            QGeoCoordinate here(m_position.latitude, m_position.longitude);
            QGeoCoordinate tgt(m_goto.latitude, m_goto.longitude);
            const double horiz = here.distanceTo(tgt);
            const double vert  = qAbs(m_position.altitude - m_goto.altitude);
            if (horiz < 2.0 && vert < 1.0) {
                m_goto.isValid = false;
                emit telemetryChanged();
            }
        }
        break;
    }
    case MAVLINK_MSG_ID_BATTERY_STATUS: {
        mavlink_battery_status_t b; mavlink_msg_battery_status_decode(&msg, &b);
        if (b.battery_remaining != INT8_MAX) m_batteryPct = b.battery_remaining;
        emit telemetryChanged(); break;
    }
    case MAVLINK_MSG_ID_VFR_HUD: {
        mavlink_vfr_hud_t v; mavlink_msg_vfr_hud_decode(&msg, &v);
        m_groundSpeed = v.groundspeed;
        m_airSpeed    = v.airspeed;
        emit telemetryChanged(); break;
    }
    case MAVLINK_MSG_ID_HOME_POSITION: {
        mavlink_home_position_t hp; mavlink_msg_home_position_decode(&msg, &hp);
        m_home.latitude  = hp.latitude / 1e7;
        m_home.longitude = hp.longitude / 1e7;
        m_home.altitude  = hp.altitude / 1000.0;
        m_home.isValid   = true;
        emit telemetryChanged(); break;
    }
    case MAVLINK_MSG_ID_COMMAND_ACK: {
        mavlink_command_ack_t ack; mavlink_msg_command_ack_decode(&msg, &ack);
        // find pending by command
        for (int i = m_pending.size()-1; i >= 0; --i) {
            if (m_pending[i].cmd == ack.command) {
                const uint8_t r = ack.result;
                if (r == MAV_RESULT_ACCEPTED) {
                    emit commandSucceeded(ack.command);
                    m_pending.removeAt(i);
                } else if (r == MAV_RESULT_IN_PROGRESS) {
                    // extend deadline
                    m_pending[i].dueAtMs = nowMs64() + m_pending[i].timeoutMs;
                } else {
                    emit commandFailed(ack.command, r);
                    m_pending.removeAt(i);
                }
                break;
            }
        }
        if (m_pending.isEmpty()) m_cmdTimer.stop();
        break;
    }
    default: break;
    }
}

void Vehicle::updateStatus() {
    if (m_lastHeartbeat.elapsed() > 3000) { m_status = "connection lost"; return; }
    if (!m_armed) m_status = "disarmed";
    else m_status = m_inAir ? "flying" : "armed";
}

void Vehicle::requestStreams() {
    // Ask for telemetry at reasonable rates (ArduPilot ok with duplicates)
    sendCommandLong(MAV_CMD_SET_MESSAGE_INTERVAL, MAVLINK_MSG_ID_GLOBAL_POSITION_INT, 1e6/10); // 10 Hz
    sendCommandLong(MAV_CMD_SET_MESSAGE_INTERVAL, MAVLINK_MSG_ID_VFR_HUD,             1e6/5);  // 5 Hz
    sendCommandLong(MAV_CMD_SET_MESSAGE_INTERVAL, MAVLINK_MSG_ID_BATTERY_STATUS,      1e6/1);  // 1 Hz
    sendCommandLong(MAV_CMD_SET_MESSAGE_INTERVAL, MAVLINK_MSG_ID_HOME_POSITION,       1e6/1);
    sendCommandLong(MAV_CMD_SET_MESSAGE_INTERVAL, MAVLINK_MSG_ID_EXTENDED_SYS_STATE,  1e6/2);
}

void Vehicle::arm(bool armIt) {
    sendCommandLongTracked(MAV_CMD_COMPONENT_ARM_DISARM, armIt ? 1.0f : 0.0f);
}

void Vehicle::takeoff(double altRel) {
    setModeText("Guided");
    sendCommandLongTracked(MAV_CMD_NAV_TAKEOFF, 0,0,0,0,0,0, static_cast<float>(altRel));
}

void Vehicle::land() {
    sendCommandLongTracked(MAV_CMD_NAV_LAND);
}

void Vehicle::rtl() {
    sendCommandLongTracked(MAV_CMD_NAV_RETURN_TO_LAUNCH);
}

void Vehicle::changeAltitude(double altRel) {
    if (!m_position.isValid) return;
    if (m_goto.isValid) {
        // keep current goto lat/lon; update target altitude (3D path)
        gotoLatLonAlt(m_goto.latitude, m_goto.longitude, altRel);
    } else {
        // hold current lat/lon; change only altitude
        gotoLatLonAlt(m_position.latitude, m_position.longitude, altRel);
    }
}

void Vehicle::setModeText(const QString& modeText) {
    uint32_t custom = m_mode; // default keep
    if      (modeText.compare("Guided", Qt::CaseInsensitive) == 0)     custom = 4;
    else if (modeText.compare("PosHold", Qt::CaseInsensitive) == 0 ||
             modeText.compare("Position Hold", Qt::CaseInsensitive) == 0) custom = 16;
    else if (modeText.compare("Brake", Qt::CaseInsensitive) == 0)      custom = 17;
    else if (modeText.compare("Land", Qt::CaseInsensitive) == 0)       custom = 9;
    else if (modeText.compare("RTL", Qt::CaseInsensitive) == 0)        custom = 6;

    // Clear goto when braking/RTL to keep UI consistent
    if (custom == 17 || custom == 6) {
        if (m_goto.isValid) { m_goto.isValid = false; emit telemetryChanged(); }
    }

    mavlink_message_t m{};
    mavlink_set_mode_t sm{};
    sm.target_system = static_cast<uint8_t>(m_sysId);
    sm.base_mode     = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
    sm.custom_mode   = custom;
    mavlink_msg_set_mode_encode(255, MAV_COMP_ID_MISSIONPLANNER, &m, &sm);
    sendMessage(m);
}

// Robust Guided GoTo using SET_POSITION_TARGET_GLOBAL_INT (position-only)
void Vehicle::gotoLatLonAlt(double lat, double lon, double altRel) {
    setModeText("Guided");

    // update UI immediately
    m_goto = GeoPoint{lat, lon, altRel, true};
    emit telemetryChanged();

    auto sendOnce = [=]() {
        mavlink_message_t m{};
        const int32_t lat_i = static_cast<int32_t>(qRound64(lat * 1e7));
        const int32_t lon_i = static_cast<int32_t>(qRound64(lon * 1e7));
        const uint16_t type_mask =
            0b0000111111111000; // ignore everything except x/y/z position
        mavlink_msg_set_position_target_global_int_pack(
            255, MAV_COMP_ID_MISSIONPLANNER, &m,
            nowMs32(),
            static_cast<uint8_t>(m_sysId), MAV_COMP_ID_AUTOPILOT1,
            MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
            type_mask,
            lat_i, lon_i, static_cast<float>(altRel),
            0,0,0, 0,0,0, 0,0);
        sendMessage(m);
    };

    // fire a few times for reliability
    sendOnce();
    QTimer::singleShot(200, this, sendOnce);
    QTimer::singleShot(400, this, sendOnce);
}

void Vehicle::sendCommandLong(uint16_t cmd,
                              float p1,float p2,float p3,float p4,float p5,float p6,float p7) {
    mavlink_message_t m{};
    mavlink_command_long_t cl{};
    cl.target_system    = static_cast<uint8_t>(m_sysId);
    cl.target_component = MAV_COMP_ID_AUTOPILOT1;
    cl.command = cmd;
    cl.confirmation = 0;
    cl.param1=p1; cl.param2=p2; cl.param3=p3; cl.param4=p4; cl.param5=p5; cl.param6=p6; cl.param7=p7;
    mavlink_msg_command_long_encode(255, MAV_COMP_ID_MISSIONPLANNER, &m, &cl);
    sendMessage(m);
}

void Vehicle::sendCommandLongTracked(uint16_t cmd,
                                     float p1,float p2,float p3,float p4,float p5,float p6,float p7,
                                     int retries, int timeoutMs) {
    mavlink_message_t m{};
    mavlink_command_long_t cl{};
    cl.target_system    = static_cast<uint8_t>(m_sysId);
    cl.target_component = MAV_COMP_ID_AUTOPILOT1;
    cl.command = cmd;
    cl.confirmation = 0;
    cl.param1=p1; cl.param2=p2; cl.param3=p3; cl.param4=p4; cl.param5=p5; cl.param6=p6; cl.param7=p7;
    mavlink_msg_command_long_encode(255, MAV_COMP_ID_MISSIONPLANNER, &m, &cl);

    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    const uint16_t len = mavlink_msg_to_send_buffer(buf, &m);
    QByteArray pkt(reinterpret_cast<const char*>(buf), len);

    PendingCmd pc;
    pc.cmd = cmd;
    pc.bytes = pkt;
    pc.retries = retries;
    pc.timeoutMs = timeoutMs;
    pc.dueAtMs = nowMs64(); // send immediately

    m_pending.push_back(pc);
    if (!m_cmdTimer.isActive()) m_cmdTimer.start();
}
