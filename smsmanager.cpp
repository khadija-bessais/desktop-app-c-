#include "smsmanager.h"
#include <QDebug>
void Sms::setTwilioCredentials(const QString &accountSid, const QString &authToken, const QString &twilioPhoneNumber)
{
    this->accountSid = accountSid;
    this->authToken = authToken;
    this->twilioPhoneNumber = twilioPhoneNumber;
}
Sms::Sms(QObject *parent) : QObject(parent)
{
    // Initialize your Twilio credentials and phone number
    accountSid = "AC7093fe078bacd39d961072ea4e16eff5";
    authToken = "b01562fdffb9d3ee32e42f0031ab59e2";
    twilioPhoneNumber = "+12563986302";


    connect(&curlProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Sms::handleSmsResponse);
}

void Sms::sendSms(const QString &to, const QString &message)
{

    qDebug() << "Before sending SMS...";
    QString fullMessage = "GlobeWing: " + message;
    QStringList arguments;
    arguments << "-X" << "POST"   // les données sont incluses dans le corps de la requête HTTP et sont envoyées au serveur pour traitement.
              << "https://api.twilio.com/2010-04-01/Accounts/" + accountSid + "/Messages.json"
              << "--data-urlencode" << "To=" + to
              << "--data-urlencode" << "From=" + twilioPhoneNumber
              << "--data-urlencode" << "Body=" + fullMessage
              << "-u" << accountSid + ":" + authToken;

    // Start the curl process
    curlProcess.start("curl", arguments);

    // Wait for the process to finish (with a timeout to prevent infinite waiting)
    if (!curlProcess.waitForFinished(3000))
    {
        // If the process didn't finish within the timeout, handle the error
        qDebug() << "Error: Timed out waiting for curl process to finish.";
    }

    // Check the exit code of the process
    int exitCode = curlProcess.exitCode();
    qDebug() << "Curl process finished with exit code:" << exitCode;

    // Handle the response based on the exit code (you might need to adjust this part)
    if (exitCode == 0)
    {
        qDebug() << "SMS sent successfully!";
    }
    else
    {
        qDebug() << "Error sending SMS. Exit code:" << exitCode;
        qDebug() << "Error details:" << curlProcess.errorString();
    }

    qDebug() << "Sending SMS completed.";
}


void Sms::handleSmsResponse()
{
    qDebug() << "Handling SMS response...";

    if (curlProcess.exitCode() == 0)
    {
        qDebug() << "SMS sent successfully!";
    }
    else
    {
        qDebug() << "Error sending SMS. Exit code:" << curlProcess.exitCode();
        qDebug() << "Error details:" << curlProcess.readAllStandardError();
    }

    // Emit the signal to indicate that SMS sending is finished
    emit finishedSending();

}
