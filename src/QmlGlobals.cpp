#include "QmlGlobals.h"
#include "link/UdpLink.h"
// Forward decl only; header not required if you don't call into it here.
#include <QDebug>

QmlGlobals::QmlGlobals(UdpLink* link, MultiVehicleManager* mvm, QObject* parent)
    : QObject(parent), m_link(link), mvm(mvm) {
    if (m_link) {
        connect(m_link, &UdpLink::udpPortChanged, this, &QmlGlobals::udpPortChanged);
    }
}

quint16 QmlGlobals::udpPort() const {
    return m_link ? m_link->udpPort() : 0;
}

void QmlGlobals::setUdpPort(int port) {
    if (!m_link) {
        qWarning() << "[QmlGlobals] UdpLink not set";
        return;
    }
    if (port <= 0 || port > 65535) {
        qWarning() << "[QmlGlobals] Invalid UDP port:" << port;
        return;
    }

    const quint16 p = static_cast<quint16>(port);
    if (p == m_link->udpPort()) {
        // No change
        return;
    }

    m_link->rebindUdpPort(p);

    // Propagate a UI reset signal for QML; alternatively, connect UdpLink::linkReset directly.
    emit linksReset();
}
