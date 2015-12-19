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

#include "weekly.h"

#define GET_REPLY \
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());\
    if(!reply) return;\
    if(reply->error() != QNetworkReply::NoError){\
        zError << tr("Request %1 error: ").arg(reply->url().toString());\
        zError << reply->errorString();\
        zErrorQuit;\
    }\


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

Weekly::Weekly(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);

    m_cookieJar = new CookieJar(this);

    m_networkManager->setCookieJar(m_cookieJar);
}

void Weekly::init(const QByteArray &email, const QByteArray &pass,
                  const QString &date, const QString &keyword,
                  bool save, bool isDefault)
{
    m_email = email;

    do{
        if(m_email.isEmpty()) {
            if(isDefault) {
                QByteArray default_email = getValueFromConfig("default_email").toByteArray();

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

    m_password = getValueFromConfig("password", m_email).toByteArray();

    if(!pass.isEmpty()) {
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
    } else if(m_password.isEmpty()){
        zPrint << tr("input password: ");

        m_password = readLineFromStdin().toUtf8();

        if(m_password.isEmpty())
            zQuit;
    }

    m_targetDate = QDate::fromString(date, DATE_FORMAT);
    m_saveCookie = save;
    m_keyword = keyword;

    if(save) {
        setValueToConfig("password", m_password, m_email);
    }

    if(isDefault) {
        setValueToConfig("default_email", m_email);
    }
}

bool Weekly::commitWeekly(const QByteArray &content_json)
{
    if(content_json.isEmpty()) {
        m_interlocutioMode = true;
    } else {
        const QJsonDocument json_doc = QJsonDocument::fromJson(content_json);

        if(!json_doc.isArray()) {
            zError << tr("Data is not json array.");

            return false;
        }

        for(const QJsonValue &value: json_doc.array()) {
            m_weeklyDataList << value.toString();
        }
    }

    QVariant cookies = getValueFromConfig("cookies", m_email).toByteArray();
    m_csrf_token = getValueFromConfig("csrf_token", m_email).toByteArray();
    m_members_id = getValueFromConfig("members_id", m_email).toByteArray();

    if(cookies.isValid() && !m_csrf_token.isEmpty() && !m_members_id.isEmpty()) {
        m_cookieJar->initCookies(cookies);

        httpRequest(&Weekly::onGetEditWeeklyPageFinished, getEditWeeklyUrl());
        /// get edit weekly page, and post weekly;
    } else {
        httpRequest(&Weekly::onInitCookieFinished, url_tower);
    }

    return true;
}

void Weekly::onInitCookieFinished()
{
    GET_REPLY

    reply->deleteLater();

    httpRequest(&Weekly::onGetLoginPageFinished, url_login_page);
    /// init csrf-token
}


void Weekly::onGetLoginPageFinished()
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

                if(m_saveCookie)
                    setValueToConfig("csrf_token", m_csrf_token, m_email);

                reply->deleteLater();

                QByteArray data = "email=" + m_email +"&password=" + m_password;

                QByteArrayMap rawHeader;

                rawHeader["X-CSRF-Token"] = m_csrf_token;

                httpRequest(&Weekly::onLoginFinished, url_login_page, data, rawHeader);
                /// login.
            }
        }
    }

    reply->deleteLater();
}

void Weekly::onLoginFinished()
{
    GET_REPLY

    const QByteArray data = reply->readAll();

    const QJsonObject &json_obj = QJsonDocument::fromJson(data).object();
    const QJsonArray &errors = json_obj["errors"].toArray();

    for(const QJsonValue &value : errors) {
        zError << (value.toObject()["msg"].toString());

        zErrorQuit;
    }

    if(!errors.isEmpty())
        qApp->exit(-1);

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
                        httpRequest(&Weekly::onGetProjectsPageFinished, tmp_list[1]);
                        /// get projects page.
                    }

                }, tmp_list[1]);
            }
        }, url_tower + target_url);
    }

    reply->deleteLater();
}

void Weekly::onGetProjectsPageFinished()
{
    GET_REPLY

    const QByteArray data = reply->readAll();

    QRegularExpression rx("/members/\\w+?(?=\\?me)");
    QRegularExpressionMatch match = rx.match(data);

    if(match.isValid()) {
        m_members_id = match.captured().toLatin1();

        if(m_saveCookie)
            setValueToConfig("members_id", m_members_id, m_email);

        httpRequest(&Weekly::onGetEditWeeklyPageFinished, getEditWeeklyUrl());
        /// get edit weekly page.
    } else {
        zError << rx.errorString();

        zErrorQuit;
    }
}

void Weekly::onGetEditWeeklyPageFinished()
{
    GET_REPLY

    const QByteArray data = reply->readAll();

    const QJsonObject &json_obj = QJsonDocument::fromJson(data).object();
    const QJsonArray &errors = json_obj["errors"].toArray();

    for(const QJsonValue &value : errors) {
        zError << value.toObject()["msg"].toString();

        zErrorQuit;
    }

    if(json_obj["success"].toBool()) {
        if(m_saveCookie) {
            QVariant cookie_list = m_cookieJar->getCookies();

            setValueToConfig("cookies", cookie_list, m_email);
        }

        const QString &html = json_obj["html"].toString();

        QRegularExpression rx("name=\"(<request_type>\\w+?)\"\\s*value=\"(<request_id>\\w+?)\".*?\\s*?(<message>.*?" + m_keyword +".*)");
        QRegularExpressionMatchIterator match = rx.globalMatch(html);

        if(match.isValid()) {
            QJsonArray json_data;

            int i = 0;

            while(match.hasNext()) {
                ++i;

                QString content;

                QRegularExpressionMatch tmp = match.next();

                if(m_interlocutioMode) {
                    zPrint.nospace() << tmp.captured("message").trimmed() << ": ";

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
                json_obj["content"] = content.replace(" ", "&nbsp;");

                json_data << json_obj;

                zDebug << "message:" << tmp.captured("message");
                zDebug << json_obj;
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
                    zPrint << "Success";

                    zQuit;
                } else {
                    zErrorQuit;
                }
            }, getPostWeeklyUrl(), request_data, rawHeader);
        } else {
            zError << rx.errorString();

            zErrorQuit;
        }
    }
}

QDate Weekly::getWeekStartDate(const QDate &date)
{
    return date.addDays(- date.dayOfWeek() + 1);
}

int Weekly::getWeekNumber(const QDate &date, int *year)
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

QByteArray Weekly::getTargetWeek() const
{
    int year;
    int week_index;

    week_index = getWeekNumber(m_targetDate, &year);

    if(week_index > 0) {
        return QByteArray::number(year) + "-" + QByteArray::number(week_index);
    }

    return "";
}

QByteArray Weekly::getPostWeeklyUrl() const
{
    return url_tower + m_members_id + "/weekly_reports/" + getTargetWeek();
}

QByteArray Weekly::getEditWeeklyUrl() const
{
    return url_tower + m_members_id + "/weekly_reports/" + getTargetWeek() + "/edit";
}

template<typename Function>
void Weekly::httpRequest(Function slot, const QByteArray &url,
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

    if(!data.isEmpty()) {
        zInfo << tr("request data:") << QString::fromUtf8(QByteArray::fromPercentEncoding(data));
    }
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
