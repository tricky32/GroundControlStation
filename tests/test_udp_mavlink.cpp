#include <QtTest>
#include <QUdpSocket>
#include "../src/link/UdpLink.h"
#include "../src/mav/MavlinkCodec.h"
#include <common/mavlink.h>

class UdpMavlinkTest: public QObject {
    Q_OBJECT
private slots:
    void test_heartbeat_rx(){
        UdpLink link; QVERIFY(link.bind(0)); // ephemeral port
        quint16 port = link.boundPort();
        MavlinkCodec codec;

        QObject::connect(&link, &UdpLink::datagramReceived, [&](const QByteArray& b, QHostAddress a, quint16 p){
            codec.feed(b, Endpoint{a,p});
        });

        bool gotHb=false;
        QObject::connect(&codec, &MavlinkCodec::messageReceived, [&](mavlink_message_t m, Endpoint){
            if(m.msgid==MAVLINK_MSG_ID_HEARTBEAT) gotHb=true;
        });

        QUdpSocket sock;
        QByteArray hb;
        {
            mavlink_message_t msg{}; mavlink_heartbeat_t payload{};
            payload.type=MAV_TYPE_QUADROTOR; payload.autopilot=MAV_AUTOPILOT_ARDUPILOTMEGA;
            payload.base_mode=0; payload.custom_mode=0; payload.system_status=MAV_STATE_STANDBY;
            mavlink_msg_heartbeat_encode(1,1,&msg,&payload);
            hb.resize(MAVLINK_MAX_PACKET_LEN);
            int n = mavlink_msg_to_send_buffer(reinterpret_cast<uint8_t*>(hb.data()), &msg);
            hb.resize(n);
        }
        sock.writeDatagram(hb, QHostAddress::LocalHost, port);

        QTRY_VERIFY_WITH_TIMEOUT(gotHb, 1000);
    }
};

QTEST_MAIN(UdpMavlinkTest)
#include "test_udp_mavlink.moc"
