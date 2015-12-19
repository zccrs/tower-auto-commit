#include <QCoreApplication>
#include <QCommandLineParser>
#include <QNetworkProxy>
#include <QDate>
#include <QRegularExpression>
#include <QFile>
#include <QProcess>
#include <QTranslator>
#include <QLibraryInfo>

#include "weekly.h"

#define MODE_INPUT "input"
#define MODE_FILE "file"
#define MODE_COMMAND "command"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setApplicationName("tower-tool");
    a.setApplicationVersion("1.0.0");

    QTranslator translator;
    translator.load(":/i18n/" + a.applicationName() + "_" + QLocale::system().name());
    a.installTranslator(&translator);
//    QNetworkProxy proxy;

//    proxy.setHostName("127.0.0.1");
//    proxy.setPort(8888);
//    proxy.setType(QNetworkProxy::HttpProxy);
//    proxy.setApplicationProxy(proxy);

    QCommandLineOption option_email(QStringList() << "e" << "email", QObject::tr("email address."), "email");
    QCommandLineOption option_pass(QStringList() << "p" << "password", QObject::tr("password."), "password");
    QCommandLineOption option_date(QStringList() << "d" << "date", QObject::tr("date, format=") + DATE_FORMAT + QObject::tr(", default is system current date."), "date");
    QCommandLineOption option_wi(QStringList() << "w" << "week-index", QObject::tr("get week index by date."));
    QCommandLineOption option_save("save", QObject::tr("save user info to local."));
    QCommandLineOption option_default("default", QObject::tr("set as default user."));
    QCommandLineOption option_clear(QStringList() << "c" << "clear", QObject::tr("clear info([user|default])."), "clear");
    QCommandLineOption option_filter(QStringList() << "f" << "filter", QObject::tr("according keyword filter weekly item(keyword format is regular expressions, default is 必填)."), "keyword");
    QCommandLineOption option_mode(QStringList() << "m" << "mode", QObject::tr("setting get weekly modes") +
                                           "\n([" MODE_INPUT "|" MODE_FILE "|" MODE_COMMAND + QObject::tr("], default=") + MODE_INPUT ")."
                                            + QObject::tr("\ninput: key in data (format=JSON)."
                                           "\nfile: the contents of the file as weekly data (text-encoding=UTF-8, format=JSON)."
                                           "\ncommand: exec command, it returns the contents as weekly data (text-encoding=UTF-8, format=JSON)."), "mode");

    option_date.setDefaultValue(QDate::currentDate().toString(DATE_FORMAT));
    option_mode.setDefaultValue(MODE_INPUT);
    option_filter.setDefaultValue("必填");

    QList<QCommandLineOption> option_list;

    option_list << option_email << option_pass << option_date << option_mode << option_filter
                << option_save << option_clear << option_default << option_wi;

    QCommandLineParser parser;

    parser.addPositionalArgument("data", QObject::tr("mode == ") + MODE_INPUT + QObject::tr("is weekly data.")
                                         + QObject::tr("\nmode == ") + MODE_FILE + QObject::tr("is file path."
                                         "\nmode == ") + MODE_COMMAND + QObject::tr("is command and arguments."));
    parser.setApplicationDescription(a.applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions(option_list);
    parser.process(a);

    if(parser.isSet(option_wi)) {
        int year;
        int week_index = QDate::fromString(parser.value(option_date), DATE_FORMAT).weekNumber(&year);

        const QString str = QString::number(year) + " " + QString::number(week_index);

        zPrint << str;

        zQuit;
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
                zError << process.errorString();

                zErrorQuit;
            }
        } else if(parser.value(option_mode) == MODE_FILE) {
            QFile file(arguments.first());

            if(file.open(QIODevice::ReadOnly)) {
                data = file.readAll();
            } else {
                zError << file.errorString();

                zErrorQuit;
            }
        } else {
            zError << QObject::tr("Unsupported %1 mode").arg(parser.value(option_mode));

            zErrorQuit;
        }

        if(weekly.commitWeekly(parser.value(option_email), parser.value(option_pass), data)) {
            return a.exec();
        }
    } else {
        if(weekly.interlocution()){
            return a.exec();
        } else {
            zErrorQuit;
        }
    }

    return 0;
}

