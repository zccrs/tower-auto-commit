#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QRegularExpression>
#include <iostream>

#include "weekly.h"

#define pError PrintError

#define GET_REPLY \
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());\
    if(!reply) return;\
    if(reply->error() != QNetworkReply::NoError){\
        PrintError::print(QString("Request %1 error: ").arg(reply->url().toString()));\
        PrintError::print(reply->errorString());\
        PrintError::print(reply->readAll());\
        qApp->quit();\
    }\

const QByteArray url_tower = "https://tower.im";
const QByteArray url_login_page = url_tower + "/users/sign_in";

Weekly::Weekly(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
}

bool Weekly::commitWeekly(const QString &email, const QString &pass, const QByteArray &content_json, const QString &date)
{
    PrintError::print(content_json);

    const QJsonDocument json_doc = QJsonDocument::fromJson(content_json);

    if(!json_doc.isArray()) {
        pError() << "Data is not json array.";

        return false;
    }

    m_email = email.toLatin1();
    m_password = pass.toLatin1();
    m_targetDate = QDate::fromString(date, "yyyy-M-d");

    for(const QJsonValue &value: json_doc.array()) {
        m_weeklyDataList << value.toString();
    }

    httpRequest(&Weekly::onInitCookieFinished, url_tower);

    return true;
}

void Weekly::onInitCookieFinished()
{
    GET_REPLY

    reply->deleteLater();

    httpRequest(&Weekly::onGetLoginPageFinished, url_login_page);
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

                reply->deleteLater();

                QByteArray data = "email=" + m_email +"&password=" + m_password;

                QByteArrayMap rawHeader;

                rawHeader["X-CSRF-Token"] = m_csrf_token;

                httpRequest(&Weekly::onLoginFinished, url_login_page, data, rawHeader);
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
        PrintError::print(value.toObject()["msg"].toString());
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

        httpRequest(&Weekly::onGetEditWeeklyPageFinished, getEditWeeklyUrl());
    } else {
        pError() << rx.errorString();
    }
}

void Weekly::onGetEditWeeklyPageFinished()
{
    GET_REPLY

    const QByteArray data = reply->readAll();

    const QJsonObject &json_obj = QJsonDocument::fromJson(data).object();
    const QJsonArray &errors = json_obj["errors"].toArray();

    for(const QJsonValue &value : errors) {
        pError() << value.toObject()["msg"].toString();
    }

    if(json_obj["success"].toBool()) {
        const QString &html = json_obj["html"].toString();

        QRegularExpression rx("name=\"(\\w+?)\"\\s*value=\"(\\w+?)\".*?\\s*?.*?必填");
        QRegularExpressionMatchIterator match = rx.globalMatch(html);

        if(match.isValid()) {
            QJsonArray json_data;
            int i = 0;

            while(match.hasNext() && i < m_weeklyDataList.count()) {
                QJsonObject json_obj;

                QRegularExpressionMatch tmp = match.next();

                json_obj[tmp.captured(1)] = tmp.captured(2);
                json_obj["content"] = m_weeklyDataList[i++];

                json_data << json_obj;
            }

            if(json_data.isEmpty()) {
                pError() << "get weekly_item_guid failed.";
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
                    pError() << value.toObject()["msg"].toString();
                }

                if(json_obj["success"].toBool()) {
                    std::cout << "Success" << std::endl;

                    qApp->quit();
                }
            }, getPostWeeklyUrl(), request_data, rawHeader);
        } else {
            pError() << rx.errorString();
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

    qDebug() << "request url:" << url << " data:" << QString::fromUtf8(QByteArray::fromPercentEncoding(data));
}


void PrintError::print(const QString &str)
{
    std::cout << qPrintable(str) << std::endl;
}

void PrintError::operator<<(const QString &str)
{
    if(str.isEmpty())
        return;

    print(str);

    qApp->exit(-1);
}

void PrintError::operator<<(const QStringList &list)
{
    if(list.isEmpty())
        return;

    for(const QString str : list) {
        print(str);
    }

    qApp->exit(-1);
}
