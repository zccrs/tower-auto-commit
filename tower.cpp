#include <iostream>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QSettings>
#include <QNetworkCookieJar>
#include <QNetworkCookie>

#include "tower.h"

#define GET_REPLY \
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());\
    if(!reply) return;\
    if(reply->error() != QNetworkReply::NoError){\
        zError << tr("Request %1 error: ").arg(reply->url().toString());\
        zError << reply->errorString();\
    }

#define CALEVENTABLE_GUID "b96e5a357a884c7e8c5c2ab12858dd02"


QString readLineFromStdin()
{
    QTextStream input_stream(stdin);

    QString str = input_stream.readLine();

    return str;
}

QVariant getValueFromConfig(const QString &key, const QString &group = "",
                            const QVariant &default_value = QVariant())
{
    QSettings settings;

    if(group.isEmpty())
        return settings.value(key, default_value);

    return settings.value(group + "/" + key, default_value);
}

void setValueToConfig(const QString key, const QVariant &value,
                      const QString &group = "")
{
    QSettings settings;

    if(group.isEmpty())
        settings.setValue(key, value);
    else
        settings.setValue(group + "/" + key, value);
}

void clearConfig(const QString &key)
{
    QSettings settings;

    settings.remove(key);
}

const QByteArray url_tower = "https://tower.im";
const QByteArray url_login_page = url_tower + "/users/sign_in";

Tower::Tower(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);

    m_cookieJar = new CookieJar(this);

    m_networkManager->setCookieJar(m_cookieJar);
}

void Tower::init(const QByteArray &email, const QByteArray &pass,
                  const QString &date, const QString &keyword, bool isDefault)
{
    m_email = email;

    do{
        if(m_email.isEmpty()) {
            if(isDefault) {
                QByteArray default_email = getValueFromConfig("default_email").toByteArray();

                zDebug << default_email;

                if(!default_email.isEmpty()) {
                    m_email = default_email;
                    break;
                }
            }

            zPrint << tr("input email: ");

            m_email = readLineFromStdin().toUtf8();

            if(m_email.isEmpty()) {
                zQuit;
            }
        }
    }while(false);

    zPrint << tr("current email is:") << m_email;

    m_targetDate = QDate::fromString(date, DATE_FORMAT);
    m_keyword = keyword;
    m_password = pass;

    if(isDefault) {
        setValueToConfig("default_email", m_email);
    }
}

void Tower::commitWeekly(const QByteArray &content_json)
{
    m_type = WeeklType;

    if(content_json.isEmpty()) {
        m_interlocutioMode = true;
    } else {
        const QJsonDocument json_doc = QJsonDocument::fromJson(content_json);

        if(!json_doc.isArray()) {
            zError << tr("Data is not json array.");

            zErrorQuit;
        }

        for(const QJsonValue &value: json_doc.array()) {
            m_weeklyDataList << value.toString();
        }
    }

    if(initCookies()) {
        postWeely();
    } else {
        httpRequest(&Tower::onInitCookieFinished, url_tower);
    }
}

void Tower::commitOvertime(const QByteArray &title, const QStringList &at_list,
                            const QByteArray &start_time, const QByteArray &end_time)
{
    QTime tmp_start_time = QTime::fromString(start_time, TIME_FORMAT);
    QTime tmp_end_time = QTime::fromString(end_time, TIME_FORMAT);

    if(!tmp_start_time.isValid()) {
        zError << tr("%1 is invaild time.").arg(QString(start_time));
        zErrorQuit;
    }

    if(!tmp_end_time.isValid()) {
        zError << tr("%1 is invaild time.").arg(QString(end_time));
        zErrorQuit;
    }

    if(tmp_end_time < tmp_start_time) {
        zError << tr("end time can not be earlier than the start time.");
        zErrorQuit;
    }

    if(tmp_end_time.minute() > 50) {
        tmp_end_time.setHMS(tmp_end_time.hour() + 1, 0, 0);
    } else if(tmp_end_time.minute() > 20) {
        tmp_end_time.setHMS(tmp_end_time.hour(), 30, 0);
    } else {
        tmp_end_time.setHMS(tmp_end_time.hour(), 0, 0);
    }

    m_type = WorkOvertimeType;
    m_overtimeTitle = title;
    m_atList = at_list;
    m_startTime = start_time;
    m_endTime = tmp_end_time.toString(TIME_FORMAT).toLatin1();

    if(initCookies()) {
        postOvertimeCalendar();
    } else {
        httpRequest(&Tower::onInitCookieFinished, url_tower);
    }
}

void Tower::onInitCookieFinished()
{
    GET_REPLY

    reply->deleteLater();

    httpRequest(&Tower::onGetLoginPageFinished, url_login_page);
    /// init csrf-token
}


void Tower::onGetLoginPageFinished()
{   
    GET_REPLY

    const QByteArray &data = reply->readAll();

    int csrf_index = data.indexOf("csrf-token");

    if(csrf_index >= 0) {
        int csrf_content_index = data.lastIndexOf("content", csrf_index);

        if(csrf_content_index >= 0) {
            const QList<QByteArray> &tmp_list = data.mid(csrf_content_index, csrf_index - csrf_content_index).split('"');

            if(tmp_list.count() > 1) {
                m_csrf_token = tmp_list[1];

                setValueToConfig("csrf_token", m_csrf_token, m_email);

                reply->deleteLater();

                /*if(!m_password.isEmpty()) {
                    if(!m_password.isEmpty() && m_password != pass) {
                        query_user:

                        zPrint << tr("Enter the password and saved locally inconsistent, whether to update the saved data? [n/Y]");

                        QString str = readLineFromStdin();

                        if(str.toUpper() == "Y" || str.isEmpty()) {
                            clearConfig(m_email);

                            m_password = pass;
                        } else if(str.toUpper() != "N") {
                            goto query_user;
                        }
                    }
                } else */
                if(m_password.isEmpty()){
                    zPrint << tr("input password: ") << "\033[8m";//\033[?25l

                    m_password = readLineFromStdin().toUtf8();

                    zPrint << "\033[m";//\033[?25h

                    if(m_password.isEmpty()) {
                        zError << tr("input psaaword is empty.");

                        zErrorQuit;
                    }
                }

                QByteArray data = "email=" + m_email +"&password=" + m_password + "&remember_me=on";

                QByteArrayMap rawHeader;

                rawHeader["X-CSRF-Token"] = m_csrf_token;

                httpRequest(&Tower::onLoginFinished, url_login_page, data, rawHeader);
                /// login.
            }
        }
    }

    reply->deleteLater();
}

void Tower::onLoginFinished()
{
    GET_REPLY

    const QByteArray data = reply->readAll();

    const QJsonObject &json_obj = QJsonDocument::fromJson(data).object();
    const QJsonArray &errors = json_obj["errors"].toArray();

    for(const QJsonValue &value : errors) {
        zError << (value.toObject()["msg"].toString());
    }

    if(!errors.isEmpty())
        zErrorQuit;

    if(json_obj["success"].toBool()) {
        const QByteArray &target_url = json_obj["target_url"].toString().toLatin1();

        httpRequest([this] {
            GET_REPLY

            const QList<QByteArray> &tmp_list = reply->readAll().split('"');

            if(tmp_list.count() > 1) {
                httpRequest([this] {
                    GET_REPLY

                    const QList<QByteArray> &tmp_list = reply->readAll().split('"');

                    if(tmp_list.count() > 1) {
                        httpRequest(&Tower::onGetProjectsPageFinished, tmp_list[1]);
                        /// get projects page.
                    }

                }, tmp_list[1]);
            }
        }, url_tower + target_url);
    }

    reply->deleteLater();
}

void Tower::onGetProjectsPageFinished()
{
    GET_REPLY;

    QVariant cookie_list = m_cookieJar->getCookies();

    setValueToConfig("cookies", cookie_list, m_email);

    const QByteArray data = reply->readAll();

    QRegularExpression rx("\"(team-guid|member-guid|member-nickname)\"\\s*value=\"(\\S+?)(?=\")");
    QRegularExpressionMatchIterator it = rx.globalMatch(data);

    if(!it.hasNext()) {
        zError << tr("get team/member guid and nickname failed: ") << rx.errorString();

        zErrorQuit;
    }

    while(it.hasNext()) {
        QRegularExpressionMatch match = it.next();

        const QString guid_type = match.captured(1);

        if(guid_type == "team-guid") {
            m_team_guid = match.captured(2).toUtf8();

            setValueToConfig("team_guid", m_team_guid, m_email);
        } else if(guid_type == "member-guid") {
            m_members_id = match.captured(2).toUtf8();

            setValueToConfig("members_id", m_members_id, m_email);
        } else if(guid_type == "member-nickname") {
            m_nickname = match.captured(2).toUtf8();

            setValueToConfig("nickname", m_nickname, m_email);
        }
    }

    switch (m_type) {
    case WeeklType:
        postWeely();
        break;
    case WorkOvertimeType:
        postOvertimeCalendar();
        break;
    default:
        zError << tr("Unknow request type: ") << m_type;
        break;
    }
}

void Tower::onGetEditWeeklyPageFinished()
{
    GET_REPLY

    const QByteArray data = reply->readAll();

    const QJsonObject &json_obj = QJsonDocument::fromJson(data).object();
    const QJsonArray &errors = json_obj["errors"].toArray();

    for(const QJsonValue &value : errors) {
        zError << value.toObject()["msg"].toString();
    }

    if(!errors.isEmpty())
        zErrorQuit;

    if(json_obj["success"].toBool()) {
        const QString &html = json_obj["html"].toString();

        QRegularExpression rx("name=\"(?<request_type>\\w+?)\"\\s*value=\"(?<request_id>\\w+?)\".*?\\s*?(?<message>.*?" + m_keyword +".*)");
        QRegularExpressionMatchIterator match = rx.globalMatch(html);

        if(!rx.isValid()) {
            zError << tr("invalid regular expression:") << rx.pattern();
            zError << rx.errorString();
            zErrorQuit;
        }

        if(match.isValid()) {
            if(!match.hasNext()) {
                zError << tr("weekly match any item not found, regular expression pattern is:");
                zPrint << rx.pattern();
                zErrorQuit;
            }

            QJsonArray json_data;

            int i = -1;

            while(match.hasNext()) {
                ++i;

                QRegularExpressionMatch tmp = match.next();

                QString message = tmp.captured("message").trimmed();
                QString content;

                if(m_interlocutioMode) {
                    zPrint.nospace() << message << tr("(Press Ctrl+D to the next step): ");

                    std::string str;

                    while (std::getline(std::cin, str)) {
                        content += QString(str.data()) + "\n";
                    }

                    std::cin.clear();

                    content.endsWith('\n');
                    content = content.left(content.count() - 1);
                } else {
                    if(i < m_weeklyDataList.count()) {
                        content = m_weeklyDataList[i];
                    } else {
                        break;
                    }
                }

                if(content.isEmpty())
                    continue;

                QJsonObject json_obj;

                json_obj[tmp.captured("request_type")] = tmp.captured("request_id");
                json_obj["content"] = content.toHtmlEscaped();

                json_data << json_obj;

                zInfo << tr("title:") << message;
                zPrint << tr("content:") << content;
            }

            if(json_data.isEmpty()) {
                zError << tr("weekly content is empty.");

                zErrorQuit;
            }

            const QByteArray request_data = "data=" + QJsonDocument(json_data).toJson(QJsonDocument::Compact).toPercentEncoding()
                                            + "&start_at=" + getWeekStartDate(m_targetDate).toString(Qt::ISODate).toLatin1();

            QByteArrayMap rawHeader;

            rawHeader["X-CSRF-Token"] = m_csrf_token;

            httpRequest([this] {
                GET_REPLY

                const QJsonObject &json_obj = QJsonDocument::fromJson(reply->readAll()).object();
                const QJsonArray &errors = json_obj["errors"].toArray();

                for(const QJsonValue &value : errors) {
                    zError << value.toObject()["msg"].toString();
                }

                if(json_obj["success"].toBool()) {
                    zPrint << tr("Success");

                    zQuit;
                } else {
                    zErrorQuit;
                }
            }, getPostWeeklyUrl(), request_data, rawHeader);
        } else {
            zError << tr("invalid regular regular expression match iterator:");
            zError << rx.errorString();
            zErrorQuit;
        }
    }
}

QDate Tower::getWeekStartDate(const QDate &date)
{
    return date.addDays(- date.dayOfWeek() + 1);
}

int Tower::getWeekNumber(const QDate &date, int *year)
{
    if(date.isValid()) {
        int week = date.dayOfWeek();

        if(week > date.dayOfYear()) {
            return getWeekNumber(getWeekStartDate(date), year);
        } else {
            if(year)
                *year = date.year();

            QDate tmp_date(date.year(), 1, 1);

            return (date.dayOfYear() - tmp_date.dayOfWeek() - 1) / 7 + 1;
        }
    }

    return 0;
}

Tower::RequestType Tower::getTypeByString(const QString &type)
{
    if(type == WEEKLY_TYPE)
        return WeeklType;
    else if(type == WORK_OVERTIME_TYPE)
        return WorkOvertimeType;
    else
        zError << tr("unknow request type:") << type;

    return Unknow;
}

QByteArray Tower::getTargetWeek() const
{
    int year;
    int week_index;

    week_index = getWeekNumber(m_targetDate, &year);

    if(week_index > 0) {
        return QByteArray::number(year) + "-" + QByteArray::number(week_index);
    }

    return "";
}

QByteArray Tower::getPostWeeklyUrl() const
{
    return url_tower + "/members/" + m_members_id + "/weekly_reports/" + getTargetWeek();
}

QByteArray Tower::getEditWeeklyUrl() const
{
    return url_tower + "/members/" + m_members_id + "/weekly_reports/" + getTargetWeek() + "/edit";
}

QByteArray Tower::getAddCalendarUrl() const
{
    return url_tower + "/teams/" + m_team_guid+ "/calendar_events/";
}

QByteArray Tower::getMembersUrl() const
{
    return url_tower + "/teams/" + m_team_guid+ "/members/";
}

QByteArray Tower::mapToHttpBody(const QByteArrayMap &map)
{
    if(map.isEmpty())
        return "";

    QByteArray array;

    for(const QByteArray &key : map.keys())
        array.append("&").append(key).append("=").append(map[key]).toPercentEncoding();

    return array.mid(1);
}

bool Tower::initCookies()
{
    QVariant cookies = getValueFromConfig("cookies", m_email).toByteArray();

    m_csrf_token = getValueFromConfig("csrf_token", m_email).toByteArray();
    m_members_id = getValueFromConfig("members_id", m_email).toByteArray();
    m_team_guid = getValueFromConfig("team_guid", m_email).toByteArray();

    if(cookies.isValid() && !m_csrf_token.isEmpty()
            && !m_members_id.isEmpty() && !m_team_guid.isEmpty()) {
        m_cookieJar->initCookies(cookies);

        return true;
    }

    return false;
}

void Tower::postWeely()
{
    httpRequest(&Tower::onGetEditWeeklyPageFinished, getEditWeeklyUrl());
    /// get edit weekly page.
}

void Tower::postOvertimeCalendar()
{
    QByteArrayMap rawHeader;

    rawHeader["X-CSRF-Token"] = m_csrf_token;

    zDebug << m_csrf_token;
    zDebug << m_cookieJar->getCookies();

    QByteArrayMap request_data;
    QByteArray current_date = m_targetDate.toString(Qt::ISODate).toLatin1();

    request_data["content"] = m_overtimeTitle;
    request_data["starts_at"] = current_date + "+" + m_startTime;
    request_data["ends_at"] = current_date + "+" + m_endTime;
    request_data["is_show_creator"] = "true";
    request_data["caleventable_type"] = "Calendar";
    request_data["caleventable_guid"] = CALEVENTABLE_GUID;

    httpRequest([this] {
        GET_REPLY;

        const QJsonObject &json_obj = QJsonDocument::fromJson(reply->readAll()).object();
        const QJsonArray &errors = json_obj["errors"].toArray();

        for(const QJsonValue &value : errors) {
            zError << value.toObject()["msg"].toString();
        }

        if(json_obj["success"].toBool()) {
            zPrint << tr("Success");

            const QByteArray url = url_tower + json_obj["url"].toString().toLatin1() + "/comments";

            const QMap<QString, QVariant> members_map = getValueFromConfig("members", m_email).toMap();

            if(members_map.isEmpty()) {
                httpRequest([this, url] {
                    GET_REPLY;

                    QRegularExpression rx("href=\"/members/(?<guid>\\w+?)\"\\s+title=\"(?<name>\\S+?)\"");

                    QRegularExpressionMatchIterator it = rx.globalMatch(reply->readAll());

                    if(!it.hasNext()) {
                        zError << tr("get member guid failed: ") << rx.errorString();

                        zErrorQuit;
                    }

                    QMap<QString, QVariant> members_map;

                    while(it.hasNext()) {
                        QRegularExpressionMatch match = it.next();

                        const QString &guid = match.captured("guid");
                        const QString &name = match.captured("name");

                        members_map[name] = guid;
                    }

                    setValueToConfig("members", members_map, m_email);

                    atMembersForOvertimeCalender(url, members_map);
                }, getMembersUrl());
            } else {
                atMembersForOvertimeCalender(url, members_map);
            }
        } else {
            zErrorQuit;
        }
    }, getAddCalendarUrl(), mapToHttpBody(request_data), rawHeader);
}

void Tower::atMembersForOvertimeCalender(const QByteArray &url, const QVariantMap &members_map)
{
    for(QString &at_name : m_atList) {
        const QString &guid = members_map[at_name].toString();

        if(guid.isEmpty()) {
            zError << tr("not found %1 user.").arg(at_name);
            continue;
        }

        m_atGuidList << guid;
        at_name = QString("<a data-mention=\"true\"  href=\"/members/%1\">%2</a>").arg(guid).arg(at_name);
    }

    QByteArrayMap request_map;

    request_map["is_html"] = "1";
    request_map["cc_guids"] = m_atGuidList.join(',').toUtf8();
    request_map["comment_content"] = m_atList.join(' ').toUtf8();

    QByteArrayMap rawHeader;

    rawHeader["X-CSRF-Token"] = m_csrf_token;

    httpRequest([this] {
        GET_REPLY;

        const QJsonObject &json_obj = QJsonDocument::fromJson(reply->readAll()).object();
        const QJsonArray &errors = json_obj["errors"].toArray();

        for(const QJsonValue &value : errors) {
            zError << value.toObject()["msg"].toString();
        }

        if(errors.isEmpty()) {
            zPrint << tr("Success");

            zQuit;
        } else {
            zErrorQuit;
        }
    }, url, mapToHttpBody(request_map), rawHeader);
}

template<typename Function>
void Tower::httpRequest(Function slot, const QByteArray &url,
                         const QByteArray &data, const QByteArrayMap &header)
{
    QNetworkRequest request;

    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (X11; Linux x86_64; rv:91.0) Gecko/20100101 Firefox/91.0");

    for(const QByteArray &key : header.keys()) {
        request.setRawHeader(key, header[key]);
    }


    QNetworkReply *reply;

    if(data.isEmpty()) {
        reply = m_networkManager->get(request);
    } else {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        reply = m_networkManager->post(request, data);
    }

    connect(reply, &QNetworkReply::finished, this, slot);

    zInfo << tr("request url:") << url;

//    if(!data.isEmpty()) {
//        zInfo << tr("request data:") << data;//QString::fromUtf8(QByteArray::fromPercentEncoding(data));
//    }
}

CookieJar::CookieJar(QObject *parent) :
    QNetworkCookieJar(parent)
{

}

QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &) const
{
    return QNetworkCookieJar::allCookies();
}

bool CookieJar::initCookies(const QVariant &cookies)
{
    QList<QNetworkCookie> cookie_list = QNetworkCookie::parseCookies(cookies.toByteArray());

    setAllCookies(cookie_list);

    return !cookie_list.isEmpty();
}

QVariant CookieJar::getCookies() const
{
    QByteArray array;

    for(const QNetworkCookie &cookie : allCookies()) {
        array.append(cookie.toRawForm(QNetworkCookie::NameAndValueOnly)).append("\n");
    }

    return array;
}
