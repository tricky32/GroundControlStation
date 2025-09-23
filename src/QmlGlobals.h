#pragma once
#include <QObject>

class UdpLink;
class MultiVehicleManager;

/**
 * QmlGlobals
 * Exposes udpPort and setUdpPort() to QML, forwarding to UdpLink.
 * Also emits linksReset() so QML (or C++) can clear UI markers immediately.
 */
class QmlGlobals : public QObject {
    Q_OBJECT
    Q_PROPERTY(quint16 udpPort READ udpPort NOTIFY udpPortChanged)

public:
    QmlGlobals(UdpLink* link, MultiVehicleManager* mvm, QObject* parent = nullptr);

    quint16 udpPort() const;

    // QML can call: QmlGlobals.setUdpPort(5761)
    Q_INVOKABLE void setUdpPort(int port);

signals:
    void udpPortChanged(quint16 port);
    void linksReset();   // for QML to react (clear overlays) if desired

private:
    UdpLink* m_link = nullptr;
    MultiVehicleManager* mvm = nullptr; // optional; left unused here to avoid assumptions
};
