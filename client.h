#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
//calendar
#include <QWidget>
#include <QDate>
#include <QSet>
#include <QMap>
#include <QList>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardItemModel>
#include <QTableView>
#include <QCalendarWidget>
#include <QSqlRecord>
#include <QFont>

class Client
{
public:
    // Constructor
    Client(QString nom = "", int cin = 0, QString prenom = "", QString telephone = "", QString sexe = "", QString resDate = "", int nbRes = 0);

    // Getters
    QString getNom() const { return NOM_CL; }
    QString getPrenom() const { return PRENOM_CL; }
    QString getTelephone() const { return TELEPHONE_CL; }
    QString getSexe() const { return SEXE_CL; }
    QString getResDate() const { return RES_DATE_CL; }
    int getId() const { return CIN_CL; }
    int getNbRes() const { return NB_RES_CL; }

    // Setters
    void setNom(const QString& n) { NOM_CL = n; }
    void setPrenom(const QString& p) { PRENOM_CL = p; }
    void setTelephone(const QString& t) { TELEPHONE_CL = t; }
    void setSexe(const QString& s) { SEXE_CL = s; }
    void setResDate(const QString& r) { RES_DATE_CL = r; }
    void setId(int id) { CIN_CL = id; }
    void setNbRes(int n) { NB_RES_CL = n; }

    // CRUD Functions
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int cin);
    bool modifier(int cin, const QString& nom, const QString& telephone, const QString& prenom, const QString& sexe, const QString& resDate, int nbRes);
    QMap<QString, QList<int>> retrieveReservationData(const QString &tableName);
    //void loadData();
    // Function to retrieve all reservation dates from the database
    QList<QMap<QString, QString>> retrieveAllCalanderDataAsList(const QString& tableName, const QDate& date);
    QList<QDate> retrieveReservationDates(const QString& tableName);

private:
    QString NOM_CL, PRENOM_CL, TELEPHONE_CL, SEXE_CL, RES_DATE_CL;
    int CIN_CL, NB_RES_CL;
};

#endif // CLIENT_H
