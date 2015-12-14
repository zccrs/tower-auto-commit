#include <QCoreApplication>
#include <QCommandLineParser>
#include <QNetworkProxy>
#include <QDate>
#include <QRegularExpression>
#include <QFile>
#include <QProcess>

#include "weekly.h"

#define MODE_INPUT "input"
#define MODE_FILE "file"
#define MODE_COMMAND "command"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setApplicationName("tower auto commit");
    a.setApplicationVersion("1.0.0");

//    QNetworkProxy proxy;

//    proxy.setHostName("127.0.0.1");
//    proxy.setPort(8888);
//    proxy.setType(QNetworkProxy::HttpProxy);
//    proxy.setApplicationProxy(proxy);

    QCommandLineOption option_email(QStringList() << "e" << "email", "email address.", "email");
    QCommandLineOption option_pass(QStringList() << "p" << "password", "password.", "password");
    QCommandLineOption option_date(QStringList() << "d" << "date", "date, format=" DATE_FORMAT ", default is system current date.", "date");
    QCommandLineOption option_wi(QStringList() << "w" << "week-index", "get week index by date.");
    QCommandLineOption option_save("save", "save user info to local.");
    QCommandLineOption option_default("default", "set as default user.");
    QCommandLineOption option_clear(QStringList() << "c" << "clear", "clear info([user|default]).", "clear");
    QCommandLineOption option_filter(QStringList() << "f" << "filter", "according keyword filter weekly item(keyword format is regular expressions, default is 必填).", "keyword");
    QCommandLineOption option_mode(QStringList() << "m" << "mode", "setting get weekly modes"
                                           "\n([" MODE_INPUT "|" MODE_FILE "|" MODE_COMMAND "], default=" MODE_INPUT ")."
                                           "\ninput: key in data (format=JSON)."
                                           "\nfile: the contents of the file as weekly data (text-encoding=UTF-8, format=JSON)."
                                           "\ncommand: exec command, it returns the contents as weekly data (text-encoding=UTF-8, format=JSON).", "mode");

    option_date.setDefaultValue(QDate::currentDate().toString(DATE_FORMAT));
    option_mode.setDefaultValue(MODE_INPUT);
    option_filter.setDefaultValue("必填");

    QList<QCommandLineOption> option_list;

    option_list << option_email << option_pass << option_date << option_mode << option_filter
                << option_save << option_clear << option_default << option_wi;

    QCommandLineParser parser;

    parser.addPositionalArgument("data", "mode == " MODE_INPUT "is weekly data."
                                         "\nmode == " MODE_FILE "is file path."
                                         "\nmode == " MODE_COMMAND "is command and arguments.");
    parser.setApplicationDescription(a.applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions(option_list);
    parser.process(a);

    if(parser.isSet(option_wi)) {
        int year;
        int week_index = QDate::fromString(parser.value(option_date), DATE_FORMAT).weekNumber(&year);

        const QString str = QString::number(year) + " " + QString::number(week_index);

        PrintError() << str;

        return 0;
    }

    Weekly weekly;

    weekly.init(parser.value(option_date), parser.value(option_filter),
                parser.isSet(option_save), parser.isSet(option_default));

    const QStringList arguments = parser.positionalArguments();

    if(!arguments.isEmpty() && parser.isSet(option_email) && parser.isSet(option_pass)) {
        QByteArray data;

        if(parser.value(option_mode) == MODE_INPUT) {
            data = arguments.first().toUtf8();
        } else if(parser.value(option_mode) == MODE_COMMAND) {
            QProcess process;

            process.start(arguments.first());

            if(process.waitForFinished()) {
                const QByteArray d = process.readAllStandardOutput();

                data = d;
            } else {
                PrintError() << process.errorString();

                return -1;
            }
        } else if(parser.value(option_mode) == MODE_FILE) {
            QFile file(arguments.first());

            if(file.open(QIODevice::ReadOnly)) {
                data = file.readAll();
            } else {
                PrintError() << file.errorString();

                return -1;
            }
        }

        if(weekly.commitWeekly(parser.value(option_email), parser.value(option_pass), data)) {
            return a.exec();
        }
    } else {
        if(weekly.interlocution()){
            return a.exec();
        } else {
            return -1;
        }
    }

    return 0;
}

