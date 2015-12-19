#ifndef WEEKLY_H
#define WEEKLY_H

#include <QObject>
#include <QVariantMap>
#include <QDate>
#include <QTextStream>

#define zDebug qDebug().noquote() << "Debug:"
#define zError qCritical().noquote() << "Error:"
#define zInfo qInfo().noquote() << "Info:"
#define zWarning qWarning().noquote() << "Warning:"
#define zPrint qDebug().noquote()
#define zExit(code) exit(code)
#define zErrorQuit zExit(-1)
#define zQuit zExit(0)

#define DATE_FORMAT "yyyy-M-d"

typedef QMap<QByteArray, QByteArray> QByteArrayMap;

class QNetworkAccessManager;
class Weekly : public QObject
{
    Q_OBJECT
public:
    explicit Weekly(QObject *parent = 0);

    void init(const QString &date, const QString &keyword, bool save, bool isDefault);
    bool commitWeekly(const QString &email, const QString &pass, const QByteArray &content_json);
    bool interlocution();

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

    QString m_keyword;

    bool m_saveCookie = true;
    bool m_default = false;
    bool m_interlocutioMode = false;
};

#endif // WEEKLY_H
