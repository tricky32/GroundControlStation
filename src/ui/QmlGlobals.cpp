#include "QmlGlobals.h"

QmlGlobals* QmlGlobals::instance() {
    static QmlGlobals g;
    return &g;
}

QmlGlobals::QmlGlobals(QObject* p)
    : QObject(p) {
    m_link.bind(m_port);

    // Wire UDP bytes -> Mavlink decoder -> Vehicle manager
    QObject::connect(&m_link, &UdpLink::datagramReceived,
                     &m_mvm,  &MultiVehicleManager::onLinkBytes);

    // Wire vehicle sends -> UDP
    QObject::connect(&m_mvm, &MultiVehicleManager::sendBytes,
                     &m_link, [&](const Endpoint& ep, const QByteArray& b){
                         m_link.send(ep.addr, ep.port, b);
                     });
}

void QmlGlobals::rebind() {
    m_link.close();
    m_link.bind(m_port);
}

void QmlGlobals::setUdpPort(int p) {
    if (m_port == p) return;
    m_port = p;
    rebind();
    emit udpPortChanged();
}
