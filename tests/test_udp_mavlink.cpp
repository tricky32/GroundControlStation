#include <QtTest>
#include <QtCore>
#include <QtNetwork>
extern "C" {
#include <common/mavlink.h>
}

#include "mav/MavlinkCodec.h"
#include "vehicle/MultiVehicleManager.h"
#include "vehicle/Vehicle.h"

// helper to pack a message into QByteArray
static QByteArray toBytes(const mavlink_message_t& m) {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    const uint16_t len = mavlink_msg_to_send_buffer(buf, &m);
    return QByteArray(reinterpret_cast<const char*>(buf), len);
}

class GcsTests : public QObject {
    Q_OBJECT
private slots:
    void codec_parses_heartbeat() {
        MavlinkCodec codec;
        int count = 0;
        QObject::connect(&codec, &MavlinkCodec::messageReceived, [&](mavlink_message_t, Endpoint){ ++count; });

        mavlink_message_t m{};
        mavlink_heartbeat_t hb{};
        hb.type = MAV_TYPE_QUADROTOR;
        hb.autopilot = MAV_AUTOPILOT_ARDUPILOTMEGA;
        mavlink_msg_heartbeat_encode(42, MAV_COMP_ID_AUTOPILOT1, &m, &hb); // sysid 42
        QByteArray bytes = toBytes(m);
        codec.feed(bytes, Endpoint{QHostAddress::LocalHost, 5760});

        QCOMPARE(count, 1);
    }

    void manager_creates_vehicle_on_first_message() {
        MultiVehicleManager mvm;

        // No vehicles initially
        QCOMPARE(mvm.vehicles().size(), 0);

        // Feed a heartbeat from sysid 7
        mavlink_message_t m{};
        mavlink_heartbeat_t hb{};
        hb.type = MAV_TYPE_QUADROTOR;
        hb.autopilot = MAV_AUTOPILOT_ARDUPILOTMEGA;
        mavlink_msg_heartbeat_encode(7, MAV_COMP_ID_AUTOPILOT1, &m, &hb);
        mvm.onMavlinkMessage(m, Endpoint{QHostAddress::LocalHost, 6000});

        // Expect one vehicle with sysId 7
        auto list = mvm.vehicles();
        QCOMPARE(list.size(), 1);
        QObject* vobj = list.first().value<QObject*>();
        QVERIFY(vobj);
        QCOMPARE(vobj->property("sysId").toInt(), 7);
    }

    void vehicle_emits_bytes_on_arm() {
        MultiVehicleManager mvm;
        QByteArray last;
        QObject::connect(&mvm, &MultiVehicleManager::sendBytes, [&](const Endpoint&, const QByteArray& b){ last = b; });

        // Create vehicle via heartbeat
        mavlink_message_t m{};
        mavlink_heartbeat_t hb{};
        mavlink_msg_heartbeat_encode(9, MAV_COMP_ID_AUTOPILOT1, &m, &hb);
        mvm.onMavlinkMessage(m, Endpoint{QHostAddress::LocalHost, 6100});

        // Get the vehicle
        auto list = mvm.vehicles();
        QObject* vobj = list.first().value<QObject*>();
        auto* v = qobject_cast<Vehicle*>(vobj);
        QVERIFY(v);

        // Arm (tracked command) should emit bytes at least once soon
        v->arm(true);
        QTRY_VERIFY(!last.isEmpty()); // waits up to default timeout for condition
    }
};

QTEST_MAIN(GcsTests)
#include "test_udp_mavlink.moc"
