#pragma once
#include <QObject>
#include <QByteArray>
#include <QHostAddress>

extern "C" {
#include <common/mavlink.h>
}

class MultiVehicleManager;

struct MavMsg {
    mavlink_message_t msg;
    QHostAddress from;
    quint16 port = 0;
};

class MavRouter : public QObject {
    Q_OBJECT
public:
    explicit MavRouter(MultiVehicleManager* mvm, QObject* parent=nullptr);

signals:
    void sendBytes(const QByteArray& data, const QHostAddress& addr, quint16 port);
    void mavMessage(const MavMsg& m);
    void mavMessageReceived(const mavlink_message_t&);

public slots:
    void onBytes(const QByteArray& bytes, const QHostAddress& from, quint16 port);


    void onCmdBytes(const QByteArray& bytes, int sysid);

public:

    void setDefaultTx(const QHostAddress& addr, quint16 port) { txAddr_ = addr; txPort_ = port; }

private:
    MultiVehicleManager* mm_ = nullptr;
    mavlink_status_t status_{};


    QHostAddress txAddr_ = QHostAddress::LocalHost;
    quint16      txPort_ = 5760;
};
