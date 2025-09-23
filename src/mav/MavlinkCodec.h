#pragma once
#include <QObject>
#include <QByteArray>
#include <QHostAddress>
#include <common/mavlink.h>

struct Endpoint { QHostAddress addr; quint16 port=0; };

class MavlinkCodec : public QObject {
    Q_OBJECT
public:
    explicit MavlinkCodec(QObject* p=nullptr):QObject(p){}
    void feed(const QByteArray& bytes, const Endpoint& ep);
    static QByteArray packMsg(const mavlink_message_t& msg);
signals:
    void messageReceived(mavlink_message_t msg, Endpoint ep);
};
