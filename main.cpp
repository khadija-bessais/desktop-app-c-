#include "mainwindow.h"
#include <QApplication>
#include "connection.h"
#include <QMessageBox>


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    Connection c;
    MainWindow w;

        bool test=c.createconnect();
        //a.setWindowIcon(QIcon(":/ressources/logo/logo.png"));
        a.setApplicationName("cleanzy");

        if(test){
            w.show();
            w.loadData();
            QMessageBox::information(nullptr, QObject::tr("database is open"),
                        QObject::tr("connection successful.\n"
                                    "Click OK."), QMessageBox::Ok);

        }
        else
            QMessageBox::critical(nullptr, QObject::tr("database is not open"),
                        QObject::tr("connection failed.\n"
                                    "Click Cancel to exit."), QMessageBox::Cancel);

    //w.showMaximized();
    return a.exec();
}
