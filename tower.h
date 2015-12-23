#ifndef TOWER_H
#define TOWER_H

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
#define zExit(code) {{QSettings settings; settings.sync();} exit(code);}
#define zErrorQuit zExit(-1)
#define zQuit zExit(0)

#define DATE_FORMAT "yyyy-M-d"
#define TIME_FORMAT "hh:mm"

#define WEEKLY_TYPE "weekly"
#define WORK_OVERTIME_TYPE "overtime"

typedef QMap<QByteArray, QByteArray> QByteArrayMap;

class CookieJar;
class QNetworkAccessManager;
class Tower : public QObject
{
    Q_OBJECT
public:
    enum RequestType{
        Unknow = -1,
        WeeklType = 0,
        WorkOvertimeType = 1
    };

    explicit Tower(QObject *parent = 0);

    void init(const QByteArray &email, const QByteArray &pass,
              const QString &date, const QString &keyword, bool isDefault);

    void commitWeekly(const QByteArray &content_json);
    void commitOvertime(const QByteArray &title, const QStringList &at_list,
                        const QByteArray &start_time, const QByteArray &end_time);

    static QDate getWeekStartDate(const QDate &date);
    static int getWeekNumber(const QDate &date, int *year);
    static RequestType getTypeByString(const QString &type);

private:
    Q_SLOT void onInitCookieFinished();
    Q_SLOT void onGetLoginPageFinished();
    Q_SLOT void onLoginFinished();
    Q_SLOT void onGetProjectsPageFinished();
    Q_SLOT void onGetEditWeeklyPageFinished();

    QByteArray getTargetWeek() const;
    QByteArray getPostWeeklyUrl() const;
    QByteArray getEditWeeklyUrl() const;
    QByteArray getAddCalendarUrl() const;
    QByteArray getMembersUrl() const;

    QByteArray mapToHttpBody(const QByteArrayMap &map);

    /// init cookies from local file.
    bool initCookies();
    /// post weekly
    void postWeely();
    /// post work overtime calendar
    void postOvertimeCalendar();
    /// at members for overtime calendar
    void atMembersForOvertimeCalender(const QByteArray &url, const QVariantMap &members_map);

    template<typename Function>
    void httpRequest(Function slot, const QByteArray &url,
                     const QByteArray &data = "",
                     const QByteArrayMap &header = QByteArrayMap());


    QNetworkAccessManager *m_networkManager;
    CookieJar *m_cookieJar;

    QByteArray m_members_id;
    QByteArray m_team_guid;
    QByteArray m_csrf_token;
    QByteArray m_nickname;

    QByteArray m_email;
    QByteArray m_password;
    QDate m_targetDate;
    QStringList m_weeklyDataList;
    QString m_keyword;

    QByteArray m_overtimeTitle;
    QStringList m_atList;
    QStringList m_atGuidList;
    QByteArray m_startTime;
    QByteArray m_endTime;

    bool m_interlocutioMode = false;

    RequestType m_type;
};

class CookieJar : public QNetworkCookieJar
{
public:
    explicit CookieJar(QObject *parent = 0);

    QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;

    bool initCookies(const QVariant &cookies);
    QVariant getCookies() const;
};

#endif // TOWER_H
