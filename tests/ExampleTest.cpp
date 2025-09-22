#include "ExampleTest.h"
#include <QSignalSpy>
#include <QTimer>
#include <QCoreApplication>

#include <mavlink.h>

void ExampleTest::init()
{
}

void ExampleTest::cleanup()
{
}

void ExampleTest::test()
{
    // Example test case
    AppConfig config;
    config.setExampleValue(42);
    QCOMPARE(config.getExampleValue(), 42);

    // Emit signal to check if it works
    QSignalSpy spy(&config, &AppConfig::exampleValueChanged);
    config.setExampleValue(100);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(config.getExampleValue(), 100);
}