#pragma once
#include <QObject>
#include "../link/UdpLink.h"
#include "../vehicle/MultiVehicleManager.h"

class QmlGlobals : public QObject {
    Q_OBJECT
    Q_PROPERTY(MultiVehicleManager* mvm READ mvm CONSTANT)
    Q_PROPERTY(int udpPort READ udpPort WRITE setUdpPort NOTIFY udpPortChanged)
public:
    static QmlGlobals* instance();
    explicit QmlGlobals(QObject* p = nullptr);

    MultiVehicleManager* mvm() { return &m_mvm; }

    int  udpPort() const { return m_port; }
    void setUdpPort(int p);

signals:
    void udpPortChanged();

private:
    void rebind();

    int m_port{5760};
    UdpLink m_link;
    MultiVehicleManager m_mvm;
};
