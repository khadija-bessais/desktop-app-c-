#include "equipement.h"


Equipement::Equipement()
{
    this->REFERENCE_EQ =0;
    this->QUANTITE_EQ = "";
    this->PRIX_EQ= "";
    this->NOM_EQ = "";
    this->MARQUE_EQ = "";
    this->TYPE_EQ = "";
    this->ETAT_EQ = "";
}

Equipement::Equipement(int REFERENCE_EQ,QString QUANTITE_EQ, QString PRIX_EQ, QString NOM_EQ , QString MARQUE_EQ, QString TYPE_EQ, QString ETAT_EQ_EQ)
{
    this->REFERENCE_EQ = REFERENCE_EQ;
    this->QUANTITE_EQ = QUANTITE_EQ;
    this->PRIX_EQ= PRIX_EQ;
    this->NOM_EQ = NOM_EQ;
    this->MARQUE_EQ = MARQUE_EQ;
    this->TYPE_EQ = TYPE_EQ;
    this->ETAT_EQ = ETAT_EQ_EQ;
}


bool Equipement::ajouter()
{
    QSqlQuery checkQuery;
    QString refStr = QString::number(REFERENCE_EQ);

    // Check if the REFERENCE already exists
    checkQuery.prepare("SELECT COUNT(*) FROM EQUIPEMENT WHERE REFERENCE_EQ = :ref");
    checkQuery.bindValue(":ref", refStr);

    if (!checkQuery.exec()) {
        qDebug() << "Error checking REFERENCE existence: " << checkQuery.lastError().text();
        return false; // Handle error appropriately
    }

    checkQuery.next(); // Move to the first (and only) row
    int count = checkQuery.value(0).toInt();

    if (count > 0) {
        // REFERENCE already exists, return false
        QMessageBox::warning(nullptr, "Duplicate REFERENCE", "La référence existe déjà dans la base de données.");
        return false;
    }

    // Proceed with the insertion
    QSqlQuery query;
    query.prepare("INSERT INTO EQUIPEMENT (REFERENCE_EQ, QUANTITE_EQ, PRIX_EQ, NOM_EQ, MARQUE_EQ, TYPE_EQ, ETAT_EQ) "
                  "VALUES (:ref, :quantite, :prix, :nom, :marque, :type, :etat)");
    query.bindValue(":ref", refStr);
    query.bindValue(":quantite", QUANTITE_EQ);
    query.bindValue(":prix", PRIX_EQ);
    query.bindValue(":nom", NOM_EQ);
    query.bindValue(":marque", MARQUE_EQ);
    query.bindValue(":type", TYPE_EQ);
    query.bindValue(":etat", ETAT_EQ);

    return query.exec();
}


QSqlQueryModel* Equipement::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT REFERENCE_EQ, QUANTITE_EQ, PRIX_EQ, NOM_EQ, MARQUE_EQ, TYPE_EQ, ETAT_EQ FROM EQUIPEMENT");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("REFERENCE_EQ"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("QUANTITE_EQ"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("PRIX_EQ"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("NOM_EQ"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("MARQUE_EQ"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("TYPE_EQ"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("ETAT_EQ"));

    return model;
}



bool Equipement::modifier(int REFERENCE_EQ, const QString& quantite, const QString& prix, const QString& nom, const QString& marque, const QString& type, const QString& etat)
{
    QSqlQuery query;

    query.prepare("UPDATE EQUIPEMENT SET QUANTITE_EQ = :quantite, PRIX_EQ = :prix, NOM_EQ = :nom, MARQUE_EQ = :marque, "
                  "TYPE_EQ = :type, ETAT_EQ = :etat WHERE REFERENCE_EQ = :ref");

    query.bindValue(":ref", REFERENCE_EQ);
    query.bindValue(":quantite", quantite);
    query.bindValue(":prix", prix);
    query.bindValue(":nom", nom);
    query.bindValue(":marque", marque);
    query.bindValue(":type", type);
    query.bindValue(":etat", etat);

    if (!query.exec()) {
        qDebug() << "Erreur lors de la mise à jour: " << query.lastError().text(); // Debugging line
        return false;
    }
    return true;
}



bool Equipement::supprimer(int reference)
{
    QSqlQuery query;
    QString refStr = QString::number(reference);
    query.prepare("DELETE FROM EQUIPEMENT WHERE REFERENCE_EQ = :ref");
    query.bindValue(":ref", refStr);
    return query.exec();
}

QMap<QString, QList<int>> Equipement::retrieveTypeData()
{
    QMap<QString, QList<int>> typeData;

    // Example SQL query to fetch equipment types (adjust the table and column names as needed)
    QSqlQuery query("SELECT TYPE_EQ FROM EQUIPEMENT");

    while (query.next()) {
        QString type = query.value(0).toString();

        // Accumulate data for each type
        if (!typeData.contains(type)) {
            typeData[type] = QList<int>();
        }
        typeData[type].append(1); // Count or process as per your logic
    }

    return typeData;
}
