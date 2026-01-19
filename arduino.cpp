#include "arduino.h"

Arduino::Arduino()
{
    data="";
    arduino_port_name="";
    arduino_is_available=false;
    serial=new QSerialPort;
}

Arduino::~Arduino(){

}

QString Arduino::getarduino_port_name()
{
    return arduino_port_name;
}

QSerialPort *Arduino::getserial()
{
   return serial;
}
int Arduino::connect_arduino()
{   // recherche du port sur lequel la carte arduino identifée par  arduino_uno_vendor_id
    // est connectée
    foreach (const QSerialPortInfo &serial_port_info, QSerialPortInfo::availablePorts()){
           if(serial_port_info.hasVendorIdentifier() && serial_port_info.hasProductIdentifier()){
               if(serial_port_info.vendorIdentifier() == arduino_uno_vendor_id && serial_port_info.productIdentifier()
                       == arduino_uno_producy_id) {
                   arduino_is_available = true;
                   arduino_port_name=serial_port_info.portName();
               } } }
        qDebug() << "arduino_port_name is :" << arduino_port_name;
        if(arduino_is_available){ // configuration de la communication ( débit...)
            serial->setPortName(arduino_port_name);
            if(serial->open(QSerialPort::ReadWrite)){
                serial->setBaudRate(QSerialPort::Baud9600); // débit : 9600 bits/s
                serial->setDataBits(QSerialPort::Data8); //Longueur des données : 8 bits,
                serial->setParity(QSerialPort::NoParity); //1 bit de parité optionnel
                serial->setStopBits(QSerialPort::OneStop); //Nombre de bits de stop : 1
                serial->setFlowControl(QSerialPort::NoFlowControl);
                return 0;
            }
            return 1;
        }
        return -1;
}

int Arduino::close_arduino()
{

    if(serial->isOpen()){
            serial->close();
            return 0;
        }
    return 1;


}


QByteArray Arduino::read_from_arduino1()
{
    if(serial->isReadable()){
         data=serial->readAll(); //récupérer les données reçues

         return data;
    }
 }

/*
QByteArray Arduino::read_from_arduino()
{
    if(serial->isReadable()){
         data.append(serial->readAll()); // append the data received

         // Check if the data contains a complete message
         int startIndex = data.indexOf("access-admin-access");
         if (startIndex != -1) {
             QByteArray message = data.mid(startIndex, 19); // assuming "notif-open" is 10 characters
             data.remove(0, startIndex + 19); // remove processed data
             return message;
         }
         int startIndex1 = data.indexOf("access-semi-access");
         if (startIndex != -1) {
             QByteArray message = data.mid(startIndex1, 18); // assuming "notif-open" is 10 characters
             data.remove(0, startIndex + 18); // remove processed data
             return message;
         }
    }
    return QByteArray(); // return an empty QByteArray if no complete message is found
}
*/

QList<QByteArray> Arduino::read_from_arduino()
{
    QList<QByteArray> messages;

    if (serial->isReadable()) {
        data.append(serial->readAll()); // append the data received

        // Debug: Print raw data received
        //qDebug() << "Raw data received:" << data;

        // Split the data into lines
        QList<QByteArray> lines = data.split('\n');

        // Process each line
        foreach (const QByteArray &line, lines) {
            if (!line.isEmpty()) {
                // Debug: Print each non-empty line
                //qDebug() << "Processing line:" << line;

                // Process the non-empty line
                messages.append(line);
            }
        }

        // Remove processed data
        data = lines.last();
    }

    return messages;
}

bool Arduino::containsSubstring(const QList<QByteArray> &messages, const QByteArray &substring)
{
    foreach (const QByteArray &message, messages) {
        // Debug: Print each message being checked
        //qDebug() << "Checking message:" << message;

        if (message.contains(substring)) {
            // Debug: Print when the substring is found
            //qDebug() << "Substring found in message:" << message;

            return true;  // Return true if the substring is found in any message
        }
    }
    return false;  // Return false if the substring is not found in any message
}

int Arduino::write_to_arduino( QByteArray d)

{

    if(serial->isWritable()){
        serial->write(d);  // envoyer des donnés vers Arduino
    }else{
        qDebug() << "Couldn't write to serial!";
    }


}
