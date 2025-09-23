#pragma once
#include <QObject>
#include "../link/UdpLink.h"
#include "../vehicle/MultiVehicleManager.h"

class QmlGlobals : public QObject {
    Q_OBJECT
    Q_PROPERTY(int udpPort READ udpPort WRITE setUdpPort NOTIFY udpPortChanged)
    Q_PROPERTY(MultiVehicleManager* mvm READ mvm CONSTANT)
public:
    explicit QmlGlobals(QObject* p=nullptr);

    int udpPort() const { return m_port; }
    void setUdpPort(int p);

    MultiVehicleManager* mvm(){ return &m_mvm; }

signals:
    void udpPortChanged();

private:
    UdpLink m_link;
    MultiVehicleManager m_mvm;
    int m_port=5760;
};
