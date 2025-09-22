#include <QtTest>
#include <QUdpSocket>

class LinkTests: public QObject {
    Q_OBJECT
private slots:
    void testLoopback() {
        QUdpSocket rx;
        QVERIFY(rx.bind(QHostAddress::LocalHost, 5761));

        QUdpSocket tx;
        QByteArray pkt("mavtest");
        QCOMPARE(tx.writeDatagram(pkt, QHostAddress::LocalHost, 5761), (qint64)pkt.size());

        QVERIFY(rx.waitForReadyRead(200));

        QByteArray buf;
        buf.resize(1500);
        QHostAddress sender;
        quint16 senderPort = 0;
        const qint64 n = rx.readDatagram(buf.data(), buf.size(), &sender, &senderPort);
        QVERIFY(n > 0);
        buf.truncate(n);

        QCOMPARE(buf, pkt);
    }
};
QTEST_MAIN(LinkTests)
#include "LinkTests.moc"
