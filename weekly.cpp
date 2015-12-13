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

    m_weeklyList << "1" << "2" << "3" << "4" << "5" << "6" << "7";
}

void Weekly::commitWeekly(const QByteArray &email, const QByteArray &pass, const QByteArray &content)
{
    m_email = email;
    m_password = pass;
    m_weekly = content;

    httpRequest(&Weekly::onInitCookieFinished, url_tower);
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
                const QByteArray &csrf = tmp_list[1];

                reply->deleteLater();

                QByteArray data = "email=" + m_email +"&password=" + m_password;

                QByteArrayMap rawHeader;

                rawHeader["X-CSRF-Token"] = csrf;

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

            while(match.hasNext() && i < m_weeklyList.count()) {
                QJsonObject json_obj;

                QRegularExpressionMatch tmp = match.next();

                json_obj[tmp.captured(1)] = tmp.captured(2);
                json_obj["content"] = m_weeklyList[i++];

                json_data << json_obj;
            }

            if(json_data.isEmpty()) {
                pError() << "get weekly_item_guid failed.";
            }

            const QByteArray request_data = "conn_guid=5c5abbcda47cad503add9cfe6e3378cd"
                                            "&report_guid=7923fb667a11480389ea97948ac40c4d"
                                            "&content=&template_guid=aefd3ef66d804c51be032d0de6018ca3"
                                            "&data=" + QJsonDocument(json_data).toJson().toPercentEncoding()
                                            + "&start_at=" + QByteArray("2014-12-29");

            httpRequest([this] {
                GET_REPLY

                qDebug() << reply->readAll();
            }, getPostWeeklyUrl(), request_data);
        } else {
            pError() << rx.errorString();
        }
    }
}

QByteArray Weekly::getTargetWeek() const
{
    int year;
    int week_index;

    if(m_targetDate.isValid()) {
        week_index = m_targetDate.weekNumber(&year);
    } else {
        week_index = QDate::currentDate().weekNumber(&year);
    }

    if(week_index > 0) {
        //return QByteArray::number(year) + "-" + QByteArray::number(week_index - 1);
        return "2014-52";
    }

    return "";
}

QByteArray Weekly::getPostWeeklyUrl() const
{
    return url_tower + m_members_id + "/weekly_reports/" + getTargetWeek();
}

QByteArray Weekly::getEditWeeklyUrl() const
{
    return url_tower + m_members_id + "/weekly_reports/" + getTargetWeek() + "/edit?""conn_guid=5c5abbcda47cad503add9cfe6e3378cd";
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

    qDebug() << "request url:" << url << " data:" << data;
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
