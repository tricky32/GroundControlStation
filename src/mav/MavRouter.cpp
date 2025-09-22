#include "MavRouter.h"
#include "vehicle/MultiVehicleManager.h"

MavRouter::MavRouter(MultiVehicleManager* mvm, QObject* parent)
    : QObject(parent), mm_(mvm) {}

void MavRouter::onBytes(const QByteArray& bytes, const QHostAddress& from, quint16 port) {
    for (unsigned char c : bytes) {
        mavlink_message_t msg;
        if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status_)) {
            MavMsg m{msg, from, port};
            emit mavMessageReceived(msg);
            emit mavMessage(m);
        }
    }
}


void MavRouter::onCmdBytes(const QByteArray& bytes, int /*sysid*/) {
    emit sendBytes(bytes, txAddr_, txPort_);
}
