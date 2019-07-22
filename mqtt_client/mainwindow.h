#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHostInfo>
#include <QHostAddress>
#include <qmqtt/qmqtt.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_lE_DomainName_returnPressed();
    void lookedUp(const QHostInfo &host);
    void on_pB_Connect_clicked();
    void on_pB_Disconnect_clicked();

    void onPublisherConnected(void);
    void onPublisherDisconnected(void);
    void on_pB_Publish_clicked();
    void onPublisherPingResp();

    void onSubscriberConnected(void);
    void onSubscriberDisconnected(void);
    void onSubscribed(const QString &topic);
    void onUnsubscribed(const QString &topic);
    void onReceived(const QMQTT::Message &message);
    void onSubscriberPingResp();
    void on_pB_Subscribe_clicked();
    void on_pB_Unsubscribe_clicked();

    void on_pushButton_clicked();
    void timeoutEvent();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    QMQTT::Client *Publisher = nullptr;
    QMQTT::Client *Subscriber = nullptr;
    void sleep(unsigned int msec);
    QString getTime(void);
    QTimer *timer;
};

#endif // MAINWINDOW_H
