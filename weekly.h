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

    bool commitWeekly(const QString &email, const QString &pass, const QByteArray &content_json, const QString &date);

    static QDate getWeekStartDate(const QDate &date);
    static int getWeekNumber(const QDate &date, int *year);

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
    QByteArray m_csrf_token;

    QByteArray m_email;
    QByteArray m_password;
    QDate m_targetDate;
    QStringList m_weeklyDataList;
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
