#ifndef EQUIPEMENT_H
#define EQUIPEMENT_H

#include <QString>
#include <QSqlQuery>
#include <QDate>
#include <QSqlQueryModel>
#include <QStandardItem>
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QPainter>

#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>

#include <QSqlRecord>

#include <QSqlError>
#include <QSqlTableModel>
//pdf
#include <QPdfWriter>
#include <QPainter>

#include <QTextDocument>

#include <QRegularExpression>

#include <QMainWindow>

#include <QDate>

class Equipement
{
public:
    Equipement();


    Equipement(int,QString,QString,QString,QString,QString,QString);


    int getreference(){return this->REFERENCE_EQ;}
    QString getquantite(){return this->QUANTITE_EQ;}
    QString getprix(){return this->PRIX_EQ;}
    QString getnom(){return this->NOM_EQ;}
    QString getmarque(){return this->MARQUE_EQ;}
    QString gettype(){return this->TYPE_EQ;}
    QString getetat(){return this->ETAT_EQ;}



    void setreference(int ref){this->REFERENCE_EQ=ref;}
    void setquantite(QString q){QUANTITE_EQ=q;}
    void setprix(QString p){PRIX_EQ=p;}
    void setnom(QString n){NOM_EQ=n;}
    void setmarque(QString m){MARQUE_EQ=m;}
    void settype(QString t){TYPE_EQ=t;}
    void setetat(QString e){ETAT_EQ=e;}



    bool sortAscending = true;

    bool ajouter();

    QSqlQueryModel * afficher();

    bool supprimer(int);

    bool modifier(int REFERENCE_EQ_EQ, const QString& QUANTITE_EQ, const QString& PRIX_EQ, const QString& NOM_EQ, const QString& MARQUE_EQ, const QString& TYPE_EQ, const QString& ETAT_EQ);

    QMap<QString, QList<int>> retrieveTypeData();  // Ensure no parameters


private:
    int REFERENCE_EQ;
    QString QUANTITE_EQ,PRIX_EQ,NOM_EQ,MARQUE_EQ,TYPE_EQ,ETAT_EQ;

};

#endif // EQUIPEMENT_H
