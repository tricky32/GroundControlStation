#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "link/UdpLink.h"
#include "mav/MavRouter.h"
#include "vehicle/MultiVehicleManager.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    auto* mvm    = new MultiVehicleManager(&app);
    auto* router = new MavRouter(mvm, &app);
    auto* link   = new UdpLink(5760, &app);

    QObject::connect(link,   &UdpLink::bytesReceived, router, &MavRouter::onBytes);
    QObject::connect(router, &MavRouter::mavMessage,  mvm,    &MultiVehicleManager::onMavMessage);

    QQmlApplicationEngine eng;
    eng.rootContext()->setContextProperty("link",   link);
    eng.rootContext()->setContextProperty("router", router);
    eng.rootContext()->setContextProperty("mvm",    mvm);

    eng.loadFromModule("gcs_application", "Main");
    if (eng.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
