#include "mainwindow.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>

static bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./test.db");
    if(!db.open()){
       return false;
    }
    else
     {
        qDebug()<<"open database success";
    }

    QSqlQuery query;
    //Attribute  0 普通货道  1 冷柜  2 热柜  3 弹簧货道
    query.exec("create table Position(PositionName varchar PRIMARY KEY,XPosition varchar,ZPosition varchar, Attribute varchar)");
    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    if(!createConnection())
    {
        qDebug()<<"open sql error";
        return 0;
    }

    return a.exec();
}
