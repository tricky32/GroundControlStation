#include "MultiVehicleManager.h"
#include "Vehicle.h"
#include "mav/MavRouter.h"
extern "C" {
#include <common/mavlink.h>

}

MultiVehicleManager::MultiVehicleManager(QObject* parent) : QObject(parent) {}

Vehicle* MultiVehicleManager::ensureVehicle(int sysid) {
    auto it = map_.find(sysid);
    if (it == map_.end()) {
        auto v = QSharedPointer<Vehicle>::create(sysid);
        map_.insert(sysid, v);
        emit vehiclesChanged();
        return v.get();
    }
    return it.value().get();
}

QVariantList MultiVehicleManager::vehicles() const {
    QVariantList out;
    for (const auto& v : map_) out << QVariant::fromValue(static_cast<QObject*>(v.get()));
    return out;
}

QObject* MultiVehicleManager::vehicleBySysId(int sysid) const {
    auto it = map_.find(sysid);
    return (it == map_.end()) ? nullptr : it.value().get();
}

void MultiVehicleManager::onMavMessage(const MavMsg& m) {
    const mavlink_message_t& msg = m.msg;
    Vehicle* v = ensureVehicle(msg.sysid);
    switch (msg.msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT: {
        mavlink_heartbeat_t hb; mavlink_msg_heartbeat_decode(&msg, &hb);
        v->setMode(QString::number(hb.custom_mode));
        v->setStatus(hb.base_mode & MAV_MODE_FLAG_SAFETY_ARMED ? "armed" : "disarmed");
    } break;
    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: {
        mavlink_global_position_int_t gp; mavlink_msg_global_position_int_decode(&msg, &gp);
        v->setPosition(QGeoCoordinate(gp.lat/1e7, gp.lon/1e7, gp.alt/1000.0));
        v->setRelAlt(gp.relative_alt/1000.0);
        v->setHeading(gp.hdg/100.0);
    } break;
    case MAVLINK_MSG_ID_VFR_HUD: {
        mavlink_vfr_hud_t hud; mavlink_msg_vfr_hud_decode(&msg, &hud);
        v->setGroundSpeed(hud.groundspeed);
        v->setAirSpeed(hud.airspeed);
    } break;
    case MAVLINK_MSG_ID_SYS_STATUS: {
        mavlink_sys_status_t s; mavlink_msg_sys_status_decode(&msg, &s);
        v->setBatteryPct(s.battery_remaining == -1 ? -1 : (int)s.battery_remaining);
    } break;
    default: break;
    }
}
