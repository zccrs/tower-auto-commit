#include <QCoreApplication>
#include <QCommandLineParser>
#include <QNetworkProxy>
#include <QDate>
#include <QRegularExpression>
#include <QFile>

#include "weekly.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    QNetworkProxy proxy;

//    proxy.setHostName("127.0.0.1");
//    proxy.setPort(8888);
//    proxy.setType(QNetworkProxy::HttpProxy);
//    proxy.setApplicationProxy(proxy);

//    QCommandLineOption option_email("-e", "指定邮箱地址");
//    QCommandLineOption option_pass("-p", "设置登录密码");
//    QCommandLineOption option_weekly("-w", "周报内容（JSON格式）");
//    QCommandLineParser parser;

//    parser.setApplicationDescription(a.applicationName());
//    parser.addHelpOption();
//    parser.addVersionOption();
//    parser.addOption(option_email);
//    parser.addOption(option_pass);
//    parser.addOption(option_weekly);
    //parser.process(a);

    Weekly weekly;

    weekly.commitWeekly("zhangjide@linuxdeepin.com", "button-yspa1314", "");

    //const QByteArray data = "name=\"report_guid\" value=\"d627de58a898480e99920254d5a751e4\"\n\n<input type=\"hidden\" name=\"report_guid\" value=\"d627de58a898480e99920254d5a751e4\" />";

//    QFile file("/home/zhang/test_tower_html.txt");

//    QByteArray data;

//    if(file.open(QIODevice::ReadOnly)) {
//        data = file.readAll();

//        //qDebug() << QString::fromUtf8(data);
//    }

//    QRegularExpression rx("name=\"(\\w+?)\"\\s*value=\"(\\w+?)\".*?\\s*?.*?必填");
//    QRegularExpressionMatchIterator match = rx.globalMatch(QString::fromUtf8(data));

//    //qDebug() << QString(data).indexOf(rx);
//    qDebug() << rx.isValid();
//    while(match.hasNext()) {
//        qDebug() << match.next().capturedTexts();
//    }

    return a.exec();
}

