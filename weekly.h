#ifndef WEEKLY_H
#define WEEKLY_H

#include <QObject>
#include <QVariantMap>
#include <QDate>

typedef QMap<QByteArray, QByteArray> QByteArrayMap;

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
    Q_SLOT void onGetProjectsPageFinished();
    Q_SLOT void onGetEditWeeklyPageFinished();


    QByteArray getTargetWeek() const;
    QByteArray getPostWeeklyUrl() const;
    QByteArray getEditWeeklyUrl() const;

    template<typename Function>
    void httpRequest(Function slot, const QByteArray &url,
                     const QByteArray &data = "",
                     const QByteArrayMap &header = QByteArrayMap());


    QNetworkAccessManager *m_networkManager;
    QByteArray m_members_id;

    QByteArray m_email;
    QByteArray m_password;
    QByteArray m_weekly;
    QDate m_targetDate;
    QStringList m_weeklyList;
};

class PrintError
{
public:
    PrintError(){}

    static void print(const QString &str);

    void operator<<(const QString &str);
    void operator<<(const QStringList &list);
};


#endif // WEEKLY_H
