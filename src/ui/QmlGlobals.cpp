#include "QmlGlobals.h"

QmlGlobals::QmlGlobals(QObject* p):QObject(p){
    m_link.bind(m_port);
    QObject::connect(&m_link, &UdpLink::datagramReceived, &m_mvm, &MultiVehicleManager::onLinkBytes);
    QObject::connect(&m_mvm, &MultiVehicleManager::sendBytes, &m_link, [&](const Endpoint& ep, const QByteArray& b){
        m_link.send(ep.addr, ep.port, b);
    });
}

void QmlGlobals::setUdpPort(int p){
    if(m_port == p) return;
    m_port = p;
    m_link.close();
    m_link.bind((quint16)m_port);
    emit udpPortChanged();
}
