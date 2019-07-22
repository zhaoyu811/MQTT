#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QEventLoop>
#include <QTimer>
#include <QDateTime>
#include <QWidget>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->dWC_MQTT->setLayout(ui->verticalLayout);
    ui->centralWidget->setLayout(ui->gridLayout_4);

//隐藏QDockWidget的标题栏
    QWidget* lTitleBar = ui->dW_MQTT->titleBarWidget();
    QWidget* lEmptyWidget = new QWidget();
    ui->dW_MQTT->setTitleBarWidget(lEmptyWidget);
    delete lTitleBar;

    //QString localHostName = QHostInfo::localHostName();
    //ui->lE_DomainName->setText(tr("主机名为：")+localHostName);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lE_DomainName_returnPressed()
{
    // 参考   https://blog.csdn.net/liang19890820/article/details/50774795
    int nID = QHostInfo::lookupHost(ui->lE_DomainName->text(), this, SLOT(lookedUp(QHostInfo)));
    Q_UNUSED(nID);
}

void MainWindow::lookedUp(const QHostInfo &host)
{
    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << host.errorString();
        return;
    }

    foreach (const QHostAddress &address, host.addresses()) {
        // 输出IPV4、IPv6地址
        if (address.protocol() == QAbstractSocket::IPv4Protocol)
        {
            qDebug() << "Found IPv4 address:" << address.toString();
            ui->lE_IP->setText(address.toString());
        }
        else if (address.protocol() == QAbstractSocket::IPv6Protocol)
            qDebug() << "Found IPv6 address:" << address.toString();
        else
            qDebug() << "Found other address:" << address.toString();
    }
}

void MainWindow::on_pB_Connect_clicked()
{
    Publisher = new QMQTT::Client(QHostAddress(ui->lE_IP->text()), ui->lE_Port->text().toUShort(), NULL);
    connect(Publisher, &QMQTT::Client::connected, this, &MainWindow::onPublisherConnected);
    Publisher->setClientId("Publisher1");
    Publisher->setUsername(ui->lE_Username->text());
    Publisher->setPassword(ui->lE_Password->text());
    Publisher->setKeepAlive(10);

    //Publisher->setAutoReconnect(true);
    //qDebug()<<"Publisher 1:"<<Publisher->autoReconnectInterval();
    //Publisher->setAutoReconnectInterval(20);
    //qDebug()<<"Publisher 2:"<<Publisher->autoReconnectInterval();

    Publisher->connectToHost();

    qDebug()<<Publisher->keepAlive();

    Subscriber = new QMQTT::Client(QHostAddress(ui->lE_IP->text()), ui->lE_Port->text().toUShort(), NULL);
    connect(Subscriber, &QMQTT::Client::connected, this, &MainWindow::onSubscriberConnected);
    Subscriber->setClientId("Subscriber1");
    Subscriber->setUsername(ui->lE_Username->text());
    Subscriber->setPassword(ui->lE_Password->text());
    Subscriber->setKeepAlive(10);

    //Subscriber->setAutoReconnect(true);     //设置了自动重新连接，断开连接后自动连接上多了一个Subscriber
    //qDebug()<<"Subscriber 1:"<<Subscriber->autoReconnectInterval();
    //Publisher->setAutoReconnectInterval(20);
    //qDebug()<<"Subscriber 2:"<<Subscriber->autoReconnectInterval();

    Subscriber->connectToHost();

    timer = new QTimer();
    timer->setInterval(10000);
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, &MainWindow::timeoutEvent);
    timer->start();
    qDebug()<<Subscriber->keepAlive();
}

void MainWindow::onPublisherConnected(void)
{
    qDebug()<<"Publisher 连接成功";
    ui->textBrowser->append(getTime()+"Publisher 连接成功");
    connect(Publisher, &QMQTT::Client::disconnected, this, &MainWindow::onPublisherDisconnected);
    connect(Publisher, &QMQTT::Client::pingresp, this, &MainWindow::onPublisherPingResp);
}

void MainWindow::onPublisherDisconnected(void)
{
    qDebug()<<"Publisher 断开连接";
    ui->textBrowser->append(getTime()+"Publisher 断开连接");
}

void MainWindow::onPublisherPingResp(void)
{
    //ui->textBrowser->append(getTime()+"Publisher ping");
}

void MainWindow::on_pB_Publish_clicked()
{
    qDebug()<<"Publisher 发布消息";
    if(Publisher == nullptr)
    {
        ui->textBrowser->append("请连接服务器");
        return;
    }
    ui->textBrowser->append(getTime()+"Publisher 发布消息"+"主题"+ui->lE_PublishTopic->text()+"消息:"+ui->lE_Message->text().toUtf8());
    QMQTT::Message message(200, ui->lE_PublishTopic->text(),\
                           ui->lE_Message->text().toUtf8());
    Publisher->publish(message);
}

void MainWindow::on_pB_Disconnect_clicked()
{
    if(Publisher == nullptr || Subscriber == nullptr)
    {
        ui->textBrowser->append("请连接服务器");
        return;
    }
    Publisher->disconnectFromHost();
    Subscriber->disconnectFromHost();
}

void MainWindow::onSubscriberConnected(void)
{
    qDebug()<<"Subscriber 连接成功";
    ui->textBrowser->append(getTime()+"Subscriber 连接成功");
    connect(Subscriber, &QMQTT::Client::disconnected, this, &MainWindow::onSubscriberDisconnected);
    connect(Subscriber, &QMQTT::Client::subscribed, this, &MainWindow::onSubscribed);
    connect(Subscriber, &QMQTT::Client::unsubscribed, this, &MainWindow::onUnsubscribed);
    connect(Subscriber, &QMQTT::Client::received, this, &MainWindow::onReceived);
    connect(Subscriber, &QMQTT::Client::pingresp, this, &MainWindow::onSubscriberPingResp);
}

void MainWindow::onSubscriberDisconnected(void)
{
    qDebug()<<"Subscriber 断开连接";
    ui->textBrowser->append(getTime()+"Subscriber 断开连接");
}

void MainWindow::onSubscribed(const QString &topic)
{
    qDebug()<<"Subscriber 订阅成功";
    ui->textBrowser->append(getTime()+"Subscriber 订阅成功"+topic);
}

void MainWindow::onUnsubscribed(const QString& topic)
{
    qDebug()<<"Subscriber 订阅取消";
    ui->textBrowser->append(getTime()+"Subscriber 订阅取消"+topic);
}

void MainWindow::onReceived(const QMQTT::Message& message)
{
    qDebug()<<"Subscriber 接收到消息";
    ui->textBrowser->append(getTime()+"Subscriber 接收到消息"+QString::fromUtf8(message.payload()));
}

void MainWindow::on_pB_Subscribe_clicked()
{
    if(Subscriber == nullptr)
    {
        ui->textBrowser->append("请连接服务器");
        return;
    }
    Subscriber->subscribe(ui->lE_SubscribeTopic->text(), 0);
}

void MainWindow::on_pB_Unsubscribe_clicked()
{
    if(Subscriber == nullptr)
    {
        ui->textBrowser->append("请连接服务器");
        return;
    }
    Subscriber->unsubscribe(ui->lE_SubscribeTopic->text());
}

void MainWindow::onSubscriberPingResp(void)
{
    //ui->textBrowser->append(getTime()+"Subscriber ping");
}

QString MainWindow::getTime(void)
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_time = current_date_time.toString("[yyyy.mm.dd hh:mm:ss.zzz] ");

    return current_time;
}

void MainWindow::on_pushButton_clicked()
{
    ui->textBrowser->clear();
}

void MainWindow::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::timeoutEvent()
{
    QString tmp = QString("Publisher: connectionState %1, isConnectedToHost %2,\
                 Subscriber: connectionState %3, isConnectedToHost %4")
                        .arg(Publisher->connectionState()).arg(Publisher->isConnectedToHost())\
                            .arg(Subscriber->connectionState()).arg(Subscriber->isConnectedToHost());
    //ui->textBrowser->append(getTime()+tmp);

    if(Publisher->isConnectedToHost() == false)
    {
        delete Publisher;
        Publisher = new QMQTT::Client(QHostAddress(ui->lE_IP->text()), ui->lE_Port->text().toUShort(), NULL);
        connect(Publisher, &QMQTT::Client::connected, this, &MainWindow::onPublisherConnected);
        Publisher->setClientId("Publisher");
        Publisher->setUsername(ui->lE_Username->text());
        Publisher->setPassword(ui->lE_Password->text());
        Publisher->setKeepAlive(10);

        //Publisher->setAutoReconnect(true);
        //qDebug()<<"Publisher 1:"<<Publisher->autoReconnectInterval();
        //Publisher->setAutoReconnectInterval(20);
        //qDebug()<<"Publisher 2:"<<Publisher->autoReconnectInterval();

        Publisher->connectToHost();

        qDebug()<<Publisher->keepAlive();
    }
    if(Subscriber->isConnectedToHost() == false)
    {
        delete Subscriber;
        Subscriber = new QMQTT::Client(QHostAddress(ui->lE_IP->text()), ui->lE_Port->text().toUShort(), NULL);
        connect(Subscriber, &QMQTT::Client::connected, this, &MainWindow::onSubscriberConnected);
        Subscriber->setClientId("Subscriber");
        Subscriber->setUsername(ui->lE_Username->text());
        Subscriber->setPassword(ui->lE_Password->text());
        Subscriber->setKeepAlive(10);

        //Subscriber->setAutoReconnect(true);     //设置了自动重新连接，断开连接后自动连接上多了一个Subscriber
        //qDebug()<<"Subscriber 1:"<<Subscriber->autoReconnectInterval();
        //Publisher->setAutoReconnectInterval(20);
        //qDebug()<<"Subscriber 2:"<<Subscriber->autoReconnectInterval();

        Subscriber->connectToHost();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    static int orderNumber=100000;
    QString cmd;
    QSqlQuery query;
    QString message = "500!1515728220!69f7013cbd2e731495e1a4db59d2bea6!2:";
    QStringList positionList;
    QStringList dropPositionList;

    orderNumber++;

    if(ui->lineEdit->text().toInt()>10)
        return;
    cmd = QString("select * from Position where XPosition='0' limit %1").arg(ui->lineEdit->text().toInt());
    if(query.exec(cmd))
    {
        while(query.next())
        {
            positionList.append(query.value(0).toString());
            dropPositionList.append(query.value(3).toString());
        }
        qDebug()<<positionList;
        qDebug()<<dropPositionList;

        for(int i=0; i<positionList.length(); i++)
        {
            cmd = QString("update Position set XPosition='1' where PositionName='%1'").arg(positionList.at(i));
            if(query.exec(cmd)==false)
            {
                ui->textBrowser->append(tr("数据库更新失败：%1").arg(positionList.at(i)));
                ui->textBrowser->append(cmd);
                return;
            }
            message+=QString("%1,").arg(orderNumber);     //订单号
            message+=(positionList.at(i)+",");            //货道
            message+="0,";      //x偏移
            message+="0,";      //Y偏移
            message+="1,";      //1件
            message+="0,";      //商品编号
            message+=(ui->lineEdit_2->text()+",");  //货柜
            message+=(dropPositionList.at(i)+";");  //放盒子位置
        }
        message.remove(message.length()-1, 1);
        QMQTT::Message message2(200, ui->lE_PublishTopic->text(),\
                               message.toUtf8());
        Publisher->publish(message2);
    }
    else
    {
        ui->textBrowser->append(tr("数据库查询失败"));
        ui->textBrowser->append(cmd);
    }
}
