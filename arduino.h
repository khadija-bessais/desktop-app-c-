#ifndef ARDUINO_H
#define ARDUINO_H
#include <QtSerialPort/QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>


class Arduino
{
public:     //méthodes de la classe Arduino
    Arduino();
    ~Arduino();
    int connect_arduino(); // permet de connecter le PC à Arduino
    int close_arduino(); // permet de femer la connexion
    int write_to_arduino( QByteArray ); // envoyer des données vers arduino
    //QByteArray read_from_arduino();  //recevoir des données de la carte Arduino
    QByteArray read_from_arduino1();
    bool containsSubstring(const QList<QByteArray> &messages, const QByteArray &substring);
    QSerialPort* getserial();   // accesseur
    QString getarduino_port_name();
    QList<QByteArray> read_from_arduino();

    int write_to_arduino1(QByteArray d);

private:
    QSerialPort * serial; //Cet objet rassemble des informations (vitesse, bits de données, etc.)
    //et des fonctions (envoi, lecture de réception,…) sur ce qu’est une voie série pour Arduino.
    static const quint16 arduino_uno_vendor_id=9025;
    static const quint16 arduino_uno_producy_id=67;
    QString arduino_port_name;
    bool arduino_is_available;
    QByteArray data;  // contenant les données lues à partir d'Arduino
};



#endif // ARDUINO_H
