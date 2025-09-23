#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "ui/QmlGlobals.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    auto* globals = new QmlGlobals(&app);
    qmlRegisterSingletonInstance("GCS", 1, 0, "QMLGL", globals);

    QQmlApplicationEngine engine;
    engine.loadFromModule("gcs_application", "Main");
    if (engine.rootObjects().isEmpty()) return -1;
    return app.exec();
}
