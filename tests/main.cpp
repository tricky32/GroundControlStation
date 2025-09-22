#include <QTest>
#include <QCoreApplication>

#include "ExampleTest.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    int result = 0;

    // run tests
    ExampleTest test;
    result |= QTest::qExec(&test, argc, argv);

    return result;
}
