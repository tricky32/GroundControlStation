#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "ui/QmlGlobals.h"
#include "vehicle/Vehicle.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    // Expose QmlGlobals singleton as QML object "QMLGL" under module GCS 1.0
    qmlRegisterSingletonInstance("GCS", 1, 0, "QMLGL", QmlGlobals::instance());

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/gcs_application/src/ui/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl) QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
