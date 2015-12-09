#include <QCoreApplication>
#include <QCommandLineParser>

#include "weekly.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineOption option_email("-e", "指定邮箱地址");
    QCommandLineOption option_pass("-p", "设置登录密码");
    QCommandLineOption option_weekly("-w", "周报内容（JSON格式）");
    QCommandLineParser parser;

    parser.setApplicationDescription(a.applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(option_email);
    parser.addOption(option_pass);
    parser.addOption(option_weekly);
    parser.process(a);

    Weekly weekly;

    weekly.commitWeekly("aaa%40qq.com", "abcdef", "");

    return a.exec();
}

