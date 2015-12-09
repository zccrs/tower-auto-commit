#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

#include "weekly.h"

const QByteArray url_tower = "https://tower.im";
const QByteArray url_login_page = url_tower + "/users/sign_in";

Weekly::Weekly(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
}

void Weekly::commitWeekly(const QByteArray &email, const QByteArray &pass, const QByteArray &content)
{
    m_email = email;
    m_password = pass;
    m_weekly = content;

    //qDebug() << email << pass << content;

    QNetworkRequest request;

    request.setUrl(QUrl(url_tower));
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;");
    request.setRawHeader("Accept", "gzip,deflate,sdch");
    request.setRawHeader("Referer", url_tower);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (X11; Linux x86_64; rv:41.0) Gecko/20100101 Firefox/41.0");

    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, &Weekly::onInitCookieFinished);
}

void Weekly::onInitCookieFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(!reply)
        return;

    reply->deleteLater();

    //qDebug() << reply->rawHeader("Cookie") << reply->readAll().count();

    QNetworkRequest request;

    request.setUrl(QUrl(url_login_page));
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;");
    request.setRawHeader("Accept", "gzip,deflate,sdch");

    reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, &Weekly::onGetLoginPageFinished);
}

void Weekly::onGetLoginPageFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(!reply)
        return;

    const QByteArray &data = reply->readAll();

    //qDebug() << "onGetLoginPageFinished()" << data;

    int csrf_index = data.indexOf("csrf-token");

    //qDebug() << csrf_index;

    if(csrf_index >= 0) {
        int csrf_content_index = data.lastIndexOf("content", csrf_index);

        //qDebug() << csrf_content_index;

        if(csrf_content_index >= 0) {
            const QByteArrayList &tmp_list = data.mid(csrf_content_index, csrf_index - csrf_content_index).split('"');

            //qDebug() << tmp_list;

            if(tmp_list.count() > 1) {
                const QByteArray &csrf = tmp_list[1];

                reply->deleteLater();

                QNetworkRequest request;

                QByteArray data = "email=" + m_email +"&password=" + m_password;

                request.setUrl(QUrl(url_login_page));
                request.setRawHeader("X-CSRF-Token", csrf);
                request.setRawHeader("Origin", url_tower);
                request.setRawHeader("X-Requested-With", "XMLHttpRequest");
                request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charser=UTF-8");
                request.setRawHeader("Referer", url_login_page);

                reply = m_networkManager->post(request, data);

                qDebug() << csrf << data;

                connect(reply, &QNetworkReply::finished, this, &Weekly::onLoginFinished);
                connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onLoginFinished()));
            }
        }
    }

    reply->deleteLater();
}

void Weekly::onLoginFinished()
{
    qDebug() << "login finished";

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(!reply)
        return;

    qDebug() << reply->errorString() << reply->readAll();

    reply->deleteLater();
}

