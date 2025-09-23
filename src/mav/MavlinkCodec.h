#pragma once
#include <QObject>
#include <QHostAddress>
#include <QByteArray>

// MAVLink
extern "C" {
#include <common/mavlink.h>
}

struct Endpoint {
    QHostAddress addr;
    quint16      port{0};
};

class MavlinkCodec : public QObject {
    Q_OBJECT
public:
    explicit MavlinkCodec(QObject* p=nullptr) : QObject(p) {}
    void feed(const QByteArray& bytes, const Endpoint& ep);

signals:
    void messageReceived(mavlink_message_t msg, Endpoint ep);
};
Q_DECLARE_METATYPE(Endpoint)
