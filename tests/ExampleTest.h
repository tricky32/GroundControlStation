#pragma once

#include "../src/AppConfig.h"

#include <QTest>


class ExampleTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test();

private:
    AppConfig appConfig;
};
