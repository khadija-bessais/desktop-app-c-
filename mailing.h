#ifndef MAILING_H
#define MAILING_H

#include <QString>
#include <QProcess>
#include <QDebug>



class Mailing
{
public:
    Mailing();
    void sendEmail(const QString &to, const QString &subject, const QString &body);


private:
    QString smtpServer = "smtp.gmail.com";
    int smtpPort = 465; // For SSL
    QString emailUser = "cashogo.tn@gmail.com";
    QString emailPassword = "sznc taqr oqzc lpjk"; // Use an app password if 2FA is enabled

};

#endif // MAILING_H
