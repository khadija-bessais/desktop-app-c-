#include "mailing.h"

Mailing::Mailing()
{

}

void Mailing::sendEmail(const QString &to, const QString &subject, const QString &body)
{
    qDebug() << "Before sending email...";

    QString fullBody = "Subject: " + subject + "\n\n" + body;
    QStringList arguments;

    // Construct curl command arguments
    arguments << "--url" << "smtps://" + smtpServer + ":" + QString::number(smtpPort)
              << "--ssl-reqd"
              << "--mail-from" << emailUser
              << "--mail-rcpt" << to
              << "--user" << emailUser + ":" + emailPassword
              << "--insecure" // Use only if necessary; better to avoid for security reasons
              << "-T" << "-" // Read email body from stdin (we will pass it via QProcess)
              << "-v"; // Verbose mode to get detailed output

    // Start the curl process
    QProcess curlProcess;
    curlProcess.start("curl", arguments);

    // Send the email body content to curl's stdin
    curlProcess.write(fullBody.toUtf8());
    curlProcess.closeWriteChannel(); // Ensure the input is complete

    // Wait for the process to finish (with a timeout)
    if (!curlProcess.waitForFinished(5000))
    {
        qDebug() << "Error: Timed out waiting for curl process to finish.";
        return;
    }

    // Check the exit code of the process
    int exitCode = curlProcess.exitCode();
    qDebug() << "Curl process finished with exit code:" << exitCode;

    // Handle the response based on the exit code
    if (exitCode == 0)
    {
        qDebug() << "Email sent successfully!";
    }
    else
    {
        qDebug() << "Error sending email. Exit code:" << exitCode;
        qDebug() << "Error details:" << curlProcess.readAllStandardError();
    }

    qDebug() << "Sending email completed.";
}
