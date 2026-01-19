#include "connection.h"

Connection::Connection() {}

bool Connection::createconnect()
{
    bool test = false;
    db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("Source_Projet2A"); // Insert your data source name
    db.setUserName("khadija"); // Insert your username
    db.setPassword("esprit18"); // Insert your password

    if (db.open()) {
        test = true;
    }

    return test;
}

void Connection::closeConnection() {
    db.close();
}



