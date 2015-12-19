#ifndef WEEKLY_H
#define WEEKLY_H

#include <QObject>
#include <QVariantMap>
#include <QDate>
#include <QTextStream>
#include <QNetworkCookieJar>

#define zCin QTextStream input_stream(std::cin); input_stream

#define zDebug qDebug().noquote() << __FILE__ << __LINE__ << __FUNCTION__ << "[Debug]:"
#define zError qCritical().noquote() << "[Error]:"
#define zInfo qInfo().noquote() << "[Info]:"
#define zWarning qWarning().noquote() << "[Warning]:"
#define zPrint qDebug().noquote()
#define zExit(code) {QSettings settings; settings.sync();} exit(code)
#define zErrorQuit zExit(-1)
#define zQuit zExit(0)

#define DATE_FORMAT "yyyy-M-d"

typedef QMap<QByteArray, QByteArray> QByteArrayMap;

class CookieJar;
class QNetworkAccessManager;
class Weekly : public QObject
{
    Q_OBJECT
public:
    explicit Weekly(QObject *parent = 0);

    void init(const QByteArray &email, const QByteArray &pass,
              const QString &date, const QString &keyword,
              bool save, bool isDefault);

    bool commitWeekly(const QByteArray &content_json);

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
    CookieJar *m_cookieJar;

    QByteArray m_members_id;
    QByteArray m_csrf_token;

    QByteArray m_email;
    QByteArray m_password;
    QDate m_targetDate;
    QStringList m_weeklyDataList;

    QString m_keyword;

    bool m_saveCookie = true;
    bool m_interlocutioMode = false;
};

class CookieJar : public QNetworkCookieJar
{
public:
    explicit CookieJar(QObject *parent = 0);

    QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;

    bool initCookies(const QVariant &cookies);
    QVariant getCookies() const;
};

#endif // WEEKLY_H
