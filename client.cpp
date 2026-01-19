#include "client.h"
#include <QSqlError>
#include <QSqlRecord>

// Constructor
Client::Client(QString nom, int cin, QString prenom, QString telephone, QString sexe, QString resDate, int nbRes)
{
    this->CIN_CL = cin;
    this->NOM_CL = nom;
    this->PRENOM_CL = prenom;
    this->TELEPHONE_CL = telephone;
    this->SEXE_CL = sexe;
    this->RES_DATE_CL = resDate;
    this->NB_RES_CL = nbRes;
}

// Add (Insert)
bool Client::ajouter()
{
    QSqlQuery checkQuery;
    QString res = QString::number(CIN_CL);

    // Check if CIN already exists
    checkQuery.prepare("SELECT COUNT(*) FROM client WHERE CIN_CL = :cin");
    checkQuery.bindValue(":cin", res);

    if (!checkQuery.exec()) {
        qDebug() << "Error checking CIN existence: " << checkQuery.lastError().text();
        return false;
    }

    checkQuery.next();
    int count = checkQuery.value(0).toInt();

    if (count > 0) {
        QMessageBox::warning(nullptr, "Duplicate CIN", "Le CIN existe déjà dans la base de données.");
        return false;
    }

    // Insert into database
    QSqlQuery query;
    query.prepare("INSERT INTO client (NOM_CL, CIN_CL, PRENOM_CL, TELEPHONE_CL, SEXE_CL, RES_DATE_CL, NB_RES_CL) "
                  "VALUES (:nom, :cin, :prenom, :telephone, :sexe, :resDate, :nbRes)");
    query.bindValue(":nom", NOM_CL);
    query.bindValue(":cin", CIN_CL);
    query.bindValue(":prenom", PRENOM_CL);
    query.bindValue(":telephone", TELEPHONE_CL);
    query.bindValue(":sexe", SEXE_CL);
    query.bindValue(":resDate", RES_DATE_CL);
    query.bindValue(":nbRes", NB_RES_CL);

    return query.exec();
}

// Display (Select)
QSqlQueryModel* Client::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT NOM_CL, CIN_CL, PRENOM_CL, TELEPHONE_CL, SEXE_CL, RES_DATE_CL, NB_RES_CL FROM client");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("CIN"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prenom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Telephone"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Sexe"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Date Reservation"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Nb Reservations"));

    return model;
}

// Delete
bool Client::supprimer(int cin)
{
    QSqlQuery query;
    query.prepare("DELETE FROM client WHERE CIN_CL = :cin");
    query.bindValue(":cin", cin);

    return query.exec();
}

// Update (Modify)
bool Client::modifier(int cin, const QString& nom, const QString& telephone, const QString& prenom, const QString& sexe, const QString& resDate, int nbRes)
{
    QSqlQuery query;
    query.prepare("UPDATE client SET NOM_CL = :nom, TELEPHONE_CL = :telephone, PRENOM_CL = :prenom, SEXE_CL = :sexe, "
                  "RES_DATE_CL = :resDate, NB_RES_CL = :nbRes WHERE CIN_CL = :cin");

    query.bindValue(":cin", cin);
    query.bindValue(":nom", nom);
    query.bindValue(":telephone", telephone);
    query.bindValue(":prenom", prenom);
    query.bindValue(":sexe", sexe);
    query.bindValue(":resDate", resDate);
    query.bindValue(":nbRes", nbRes);

    if (!query.exec()) {
        qDebug() << "Error updating client: " << query.lastError().text();
        return false;
    }

    return true;
}

QMap<QString, QList<int>> Client::retrieveReservationData(const QString &tableName) {
    QMap<QString, QList<int>> reservationData;

    // Query to get the number of reservations for each client
    QSqlQuery query;
    if (!query.exec("SELECT NB_RES_CL FROM " + tableName)) {
        qDebug() << "Error executing query:" << query.lastError().text();
        return reservationData;
    }

    while (query.next()) {
        int numReservations = query.value(0).toInt();

        // Categorize clients based on their number of reservations
        if (numReservations >= 1 && numReservations <= 5) {
            reservationData["Low (1-5)"].append(1);
        } else if (numReservations >= 6 && numReservations <= 10) {
            reservationData["Medium (6-10)"].append(1);
        } else if (numReservations >= 11) {
            reservationData["High (11+)"].append(1);
        }
    }

    qDebug() << "Reservation data retrieved successfully!";
    return reservationData;
}

//calendar


QList<QMap<QString, QString>> Client::retrieveAllCalanderDataAsList(const QString& tableName, const QDate& date)
{
    QList<QMap<QString, QString>> dataList;
    QString dateString = date.toString("yyyy-MM-dd");

    QSqlQuery query;
    QString queryString = QString("SELECT NOM_CL, CIN_CL, PRENOM_CL, TELEPHONE_CL, SEXE_CL, RES_DATE_CL, NB_RES_CL FROM %1 WHERE RES_DATE_CL = '%2'").arg(tableName, dateString);

    if (!query.exec(queryString)) {
        qDebug() << "Error executing query:" << query.lastError().text();
        return dataList;
    }

    while (query.next()) {
        QMap<QString, QString> rowData;
        QSqlRecord record = query.record();

        for (int i = 0; i < record.count(); ++i) {
            rowData[record.fieldName(i).toLower()] = query.value(i).toString();
        }

        dataList.append(rowData);
    }

    qDebug() << "Data retrieved successfully!";
    return dataList;
}

QList<QDate> Client::retrieveReservationDates(const QString& tableName)
{
    QList<QDate> reservationDates;

    QSqlQuery query;
    QString queryString = QString("SELECT DISTINCT RES_DATE_CL FROM %1").arg(tableName);

    if (!query.exec(queryString)) {
        qDebug() << "Error executing query for reservation dates:" << query.lastError().text();
        return reservationDates;
    }

    while (query.next()) {
        reservationDates.append(query.value(0).toDate());
    }

    qDebug() << "Reservation dates retrieved successfully!";
    return reservationDates;
}


