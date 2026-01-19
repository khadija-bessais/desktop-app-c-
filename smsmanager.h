#ifndef SMSMANAGER_H
#define SMSMANAGER_H
#include <QObject>
#include <QProcess>

class Sms : public QObject
{
    Q_OBJECT

public:
    explicit Sms(QObject *parent = nullptr);

    void sendSms(const QString &to, const QString &message);
    void setTwilioCredentials(const QString &accountSid, const QString &authToken, const QString &twilioPhoneNumber);

signals:
    void finishedSending();

private slots:
    void handleSmsResponse();

private:
    QProcess curlProcess;

    // Your Twilio credentials and phone number
    QString accountSid;
    QString authToken;
    QString twilioPhoneNumber;
};


#endif // SMSMANAGER_H
