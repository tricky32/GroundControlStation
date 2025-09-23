#include "Vehicle.h"
#include <QtCore/QDebug>
#include <QtMath>

// --- helpers ---
static inline uint32_t nowMs32() {
    return static_cast<uint32_t>(QDateTime::currentMSecsSinceEpoch() & 0xFFFFFFFFu);
}

Vehicle::Vehicle(int sysId, const Endpoint& ep, QObject* parent)
    : QObject(parent), m_sysId(sysId), m_ep(ep) {
    m_lastHeartbeat.start();
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
        mavlink_heartbeat_t hb;
        mavlink_msg_heartbeat_decode(&msg, &hb);
        m_mode = hb.custom_mode;
        m_armed = (hb.base_mode & MAV_MODE_FLAG_SAFETY_ARMED);
        updateStatus();
        emit telemetryChanged();
        break;
    }
    case MAVLINK_MSG_ID_EXTENDED_SYS_STATE: {
        mavlink_extended_sys_state_t s;
        mavlink_msg_extended_sys_state_decode(&msg, &s);
        m_inAir = (s.landed_state == MAV_LANDED_STATE_IN_AIR || s.landed_state == MAV_LANDED_STATE_TAKEOFF);
        updateStatus();
        emit telemetryChanged();
        break;
    }
    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: {
        mavlink_global_position_int_t gp;
        mavlink_msg_global_position_int_decode(&msg, &gp);
        m_position.latitude  = gp.lat / 1e7;
        m_position.longitude = gp.lon / 1e7;
        m_position.altitude  = gp.relative_alt / 1000.0; // meters rel
        m_position.isValid   = true;
        m_altAMSL            = gp.alt / 1000.0;
        m_heading            = gp.hdg / 100.0;
        emit telemetryChanged();
        break;
    }
    case MAVLINK_MSG_ID_BATTERY_STATUS: {
        mavlink_battery_status_t b;
        mavlink_msg_battery_status_decode(&msg, &b);
        if (b.battery_remaining != INT8_MAX)
            m_batteryPct = b.battery_remaining;
        emit telemetryChanged();
        break;
    }
    case MAVLINK_MSG_ID_VFR_HUD: {
        mavlink_vfr_hud_t v;
        mavlink_msg_vfr_hud_decode(&msg, &v);
        m_groundSpeed = v.groundspeed;
        m_airSpeed    = v.airspeed;
        emit telemetryChanged();
        break;
    }
    case MAVLINK_MSG_ID_HOME_POSITION: {
        mavlink_home_position_t hp;
        mavlink_msg_home_position_decode(&msg, &hp);
        m_home.latitude  = hp.latitude / 1e7;
        m_home.longitude = hp.longitude / 1e7;
        m_home.altitude  = hp.altitude / 1000.0;
        m_home.isValid   = true;
        emit telemetryChanged();
        break;
    }
    default:
        break;
    }
}

void Vehicle::updateStatus() {
    if (m_lastHeartbeat.elapsed() > 3000) {
        m_status = "connection lost";
        return;
    }
    if (!m_armed) {
        m_status = "disarmed";
    } else {
        m_status = m_inAir ? "flying" : "armed";
    }
}

void Vehicle::requestStreams() {
    // ask for useful telemetry (ArduPilot responds even if duplicate)
    sendCommandLong(MAV_CMD_SET_MESSAGE_INTERVAL, MAVLINK_MSG_ID_GLOBAL_POSITION_INT, 1e6/10); // 10 Hz
    sendCommandLong(MAV_CMD_SET_MESSAGE_INTERVAL, MAVLINK_MSG_ID_VFR_HUD,             1e6/5);  // 5 Hz
    sendCommandLong(MAV_CMD_SET_MESSAGE_INTERVAL, MAVLINK_MSG_ID_BATTERY_STATUS,      1e6/1);  // 1 Hz
    sendCommandLong(MAV_CMD_SET_MESSAGE_INTERVAL, MAVLINK_MSG_ID_HOME_POSITION,       1e6/1);
    sendCommandLong(MAV_CMD_SET_MESSAGE_INTERVAL, MAVLINK_MSG_ID_EXTENDED_SYS_STATE,  1e6/2);
}

void Vehicle::arm(bool armIt) {
    sendCommandLong(MAV_CMD_COMPONENT_ARM_DISARM, armIt ? 1.0f : 0.0f);
}

void Vehicle::takeoff(double altRel) {
    setModeText("Guided");
    sendCommandLong(MAV_CMD_NAV_TAKEOFF, 0, 0, 0, 0, 0, 0, static_cast<float>(altRel));
}

void Vehicle::land() {
    sendCommandLong(MAV_CMD_NAV_LAND);
}

void Vehicle::rtl() {
    sendCommandLong(MAV_CMD_NAV_RETURN_TO_LAUNCH);
}

void Vehicle::changeAltitude(double altRel) {
    // NED local setpoint would be ideal; here we reuse global setpoint at current lat/lon
    if (!m_position.isValid) return;
    gotoLatLonAlt(m_position.latitude, m_position.longitude, altRel);
}

void Vehicle::setModeText(const QString& modeText) {
    // minimal set: switch to GUIDED/LOITER/BRAKE/LAND/RTL via CUSTOM_MODE for ArduCopter
    uint32_t custom = m_mode; // default keep
    if      (modeText.compare("Guided", Qt::CaseInsensitive) == 0)     custom = 4;
    else if (modeText.compare("PosHold", Qt::CaseInsensitive) == 0 ||
             modeText.compare("Position Hold", Qt::CaseInsensitive) == 0) custom = 16;
    else if (modeText.compare("Brake", Qt::CaseInsensitive) == 0)      custom = 17;
    else if (modeText.compare("Land", Qt::CaseInsensitive) == 0)       custom = 9;
    else if (modeText.compare("RTL", Qt::CaseInsensitive) == 0)        custom = 6;

    mavlink_message_t m{};
    mavlink_set_mode_t sm{};
    sm.target_system = static_cast<uint8_t>(m_sysId);
    sm.base_mode     = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
    sm.custom_mode   = custom;
    mavlink_msg_set_mode_encode(255, MAV_COMP_ID_MISSIONPLANNER, &m, &sm);
    sendMessage(m);
}

// --- Robust Guided GoTo using SET_POSITION_TARGET_GLOBAL_INT (position-only) ---
void Vehicle::gotoLatLonAlt(double lat, double lon, double altRel) {
    setModeText("Guided");

    // show target on map immediately
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

    // fire a few times for reliability (ArduPilot will accept duplicates)
    sendOnce();
    QTimer::singleShot(200, this, sendOnce);
    QTimer::singleShot(400, this, sendOnce);
}

// --- COMMAND_LONG helper (your existing retry/ACK code can stay) ---
void Vehicle::sendCommandLong(uint16_t cmd, float p1,float p2,float p3,float p4,float p5,float p6,float p7) {
    mavlink_message_t m{};
    mavlink_command_long_t cl{};
    cl.target_system    = static_cast<uint8_t>(m_sysId);
    cl.target_component = MAV_COMP_ID_AUTOPILOT1;
    cl.command = cmd;
    cl.confirmation = 0;
    cl.param1=p1; cl.param2=p2; cl.param3=p3; cl.param4=p4; cl.param5=p5; cl.param6=p6; cl.param7=p7;
    mavlink_msg_command_long_encode(255, MAV_COMP_ID_MISSIONPLANNER, &m, &cl);
    sendMessage(m);
    // (optional) add your resend/ACK tracking here if you implemented it
}
