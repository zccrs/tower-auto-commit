#ifndef WEEKLY_H
#define WEEKLY_H

#include <QObject>

class QNetworkAccessManager;
class Weekly : public QObject
{
    Q_OBJECT
public:
    explicit Weekly(QObject *parent = 0);

    void commitWeekly(const QByteArray &email, const QByteArray &pass, const QByteArray &content);

private:
    Q_SLOT void onInitCookieFinished();
    Q_SLOT void onGetLoginPageFinished();
    Q_SLOT void onLoginFinished();

    QNetworkAccessManager *m_networkManager;
    QByteArray m_email;
    QByteArray m_password;
    QByteArray m_weekly;
};

#endif // WEEKLY_H
