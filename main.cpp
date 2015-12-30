#include <QCoreApplication>
#include <QCommandLineParser>
#include <QNetworkProxy>
#include <QDate>
#include <QRegularExpression>
#include <QFile>
#include <QProcess>
#include <QTranslator>
#include <QLibraryInfo>
#include <QSettings>

#include "tower.h"

#define zValueError(option) zError << qApp->arguments().join(' ') + QString(": %1 is unknow parameter.").arg(parser.value(option)); parser.showHelp(-1);

int main(int argc, char *argv[])
{
//    QFile file("/home/zhang/tmp.txt");

//    if(file.open(QIODevice::ReadOnly)) {
//        QRegularExpression rx("\"(team-guid|member-guid|member-nickname)\"\\s*value=\"(\\S+?)(?=\")");
//        QRegularExpressionMatchIterator it = rx.globalMatch(QString::fromUtf8(file.readAll()));

//        while(it.hasNext()) {
//            QRegularExpressionMatch match = it.next();

//            zDebug << match.capturedTexts();
//        }

//        return 0;
//    }

    if(argc > 2) {
        for (int a = 1; a < argc; ++a) {
            if(QString::fromLocal8Bit(argv[a]) == "--exec") {
                if(a < argc - 1) {
                    QSettings settings("zccrs", "tower-tool");
                    QString name = QString::fromLocal8Bit(argv[a + 1]);

                    QStringList list = settings.value("record/" + name).toStringList();

                    if(list.isEmpty()) {
                        zError << QObject::tr("not found %1 record.").arg(name);
                    } else {
                        argc = list.count();

                        for(a = 1; a < list.count(); ++a) {
                            const QString &str = list[a];

                            char *arg = new char[strlen(str.toLocal8Bit().constData()) + 1];

                            strcpy(arg, str.toLocal8Bit().constData());

                            argv[a] = arg;
                        }
                    }
                }

                break;
            }
        }
    }

    QCoreApplication a(argc, argv);

    a.setApplicationName("tower-tool");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("zccrs");

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
    QCommandLineOption option_date(QStringList() << "d" << "date", QObject::tr("date(format=yyyy-M-d), default is system current date."), "date");
    QCommandLineOption option_wi(QStringList() << "w" << "week-index", QObject::tr("get week index by date."));
    QCommandLineOption option_filter(QStringList() << "f" << "filter", QObject::tr("according keyword filter weekly item(keyword format is regular expressions, default is 必填)."), "keyword");
    QCommandLineOption option_mode("mode", QObject::tr("input weekly mode([input|file|command], we will use dialogue mode is not set)."), "mode");
    QCommandLineOption option_default("default", QObject::tr("if you specify the email then set as default user, else use the default user as a email value."));
    QCommandLineOption option_clear("clear", QObject::tr("clear info([user|default|record|all])."
                                                         "\n--clear=user: clear user info."
                                                         "\n--clear=default: clear default email."
                                                         "\n--clear=all: clear all config(contain record)."
                                                         "\n--clear=record: clear record, if not set record then clear all record."), "clear");
    QCommandLineOption option_record("record", QObject::tr("record this command and assign a name, if the name exists  then replace command."), "name");
    QCommandLineOption option_exec("exec", QObject::tr("execute recorded command."), "name");
    QCommandLineOption option_start_time(QStringList() << "st" << "start-time", QObject::tr("to work overtime start time(format=hh:mm, default is 17:30)."), "time");
    QCommandLineOption option_end_time(QStringList() << "et" << "end-time", QObject::tr("to work overtime end time(format=hh:mm, default is system current time)."), "time");
    QCommandLineOption option_title("title", QObject::tr("to work overtime request title, default is 加班申请."), "title");
    QCommandLineOption option_type(QStringList() << "rt" << "request-type", QObject::tr("type of this request([weekly|overtime, default is weekly])."
                                                                                        "\n--rt=weekly: post weekly request."
                                                                                        "\n--rt=overtime: post to work overtime request."), "type");

    option_date.setDefaultValue(QDate::currentDate().toString(DATE_FORMAT));
    option_filter.setDefaultValue("必填");
    option_start_time.setDefaultValue("17:30");
    option_end_time.setDefaultValue(QTime::currentTime().toString(TIME_FORMAT));
    option_title.setDefaultValue("加班申请");
    option_type.setDefaultValue("weekly");

    QList<QCommandLineOption> option_list;

    option_list << option_email << option_pass << option_date << option_mode << option_filter
                << option_clear << option_default << option_wi << option_record
                << option_exec << option_start_time << option_end_time << option_title << option_type;

    QCommandLineParser parser;

    parser.addPositionalArgument(QObject::tr("weekly data"), QObject::tr("--mode=input: weekly data(format=JSON)."
                                                     "\n--mode=file: file path(text-encoding=UTF-8, format=JSON)."
                                                     "\n--mode=command: command and arguments(text-encoding=UTF-8, format=JSON)."));
    parser.addPositionalArgument(QObject::tr("at target"), QObject::tr("when --rt=overtime @ user list."));

    parser.setApplicationDescription(a.applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions(option_list);
    parser.process(a);

    if(a.arguments().count() <= 1) {
        parser.showHelp(0);
    } else {
        bool match = false;

        for(const QString &str : a.arguments()) {
            if(str.startsWith('-')) {
                match = true;
                break;
            }
        }

        if(!match) {
            parser.showHelp(-1);
        }
    }

    if(parser.isSet(option_exec)) {
        zValueError(option_exec);
    }

    if(parser.isSet(option_wi)) {
        int year;
        int week_index = QDate::fromString(parser.value(option_date), DATE_FORMAT).weekNumber(&year);

        const QString str = QString::number(year) + " " + QString::number(week_index);

        zPrint << str;

        zQuit;
    }

    if(parser.isSet(option_clear)) {
        const QString value = parser.value(option_clear);

        QSettings settings;

        if(value == "user") {
            if(parser.isSet(option_email)) {
                settings.remove(parser.value(option_email));
            } else {
                zError << QObject::tr("email unspecified.");

                zErrorQuit;
            }
        } else if(value == "default") {
            settings.remove("default_email");
        } else if(value == "all") {
            settings.clear();
        } else if(value == "record") {
            if(parser.isSet(option_record)) {
                settings.remove("record/" + parser.value(option_record));
            } else {
                query_user:

                zPrint << QObject::tr("clear all record? [n/Y]");

                QTextStream input_stream(stdin);
                QString str = input_stream.readLine();

                if(str.toUpper() == "Y" || str.isEmpty()) {
                    settings.remove("record");
                } else if(str.toUpper() != "N") {
                    goto query_user;
                }
            }
        } else {
            zValueError(option_clear);
        }

        zQuit;
    }

    if(parser.isSet(option_record)) {
        QSettings settings;

        QStringList argv = a.arguments();

        int index = argv.indexOf("--record");

        if(index >= 0) {
            argv.removeAt(index);

            if(index < argv.count())
                argv.removeAt(index);
        }

        QStringList list = settings.value("record/" + parser.value(option_record)).toStringList();

        if(!list.isEmpty()) {
            zWarning << QObject::tr("Will be \"%1\" replacement for the \"%2\".").arg(argv.join(' ')).arg(list.join(' '));
        }

        settings.setValue("record/" + parser.value(option_record), argv);
    }

    Tower::RequestType request_type = Tower::getTypeByString(parser.value(option_type));

    if(request_type == Tower::Unknow)
        zErrorQuit;

    const QStringList arguments = parser.positionalArguments();

    switch (request_type) {
    case Tower::WeeklType:{
        QByteArray data;

        if(!arguments.isEmpty() && parser.isSet(option_mode)) {
            if(parser.value(option_mode) == "input") {
                data = arguments.first().toUtf8();
            } else if(parser.value(option_mode) == "command") {
                QProcess process;

                process.start(arguments.first());

                if(process.waitForFinished()) {
                    const QByteArray d = process.readAllStandardOutput();

                    data = d;
                } else {
                    zError << process.errorString();

                    zErrorQuit;
                }
            } else if(parser.value(option_mode) == "file") {
                QFile file(arguments.first());

                if(file.open(QIODevice::ReadOnly)) {
                    data = file.readAll();
                } else {
                    zError << file.errorString();

                    zErrorQuit;
                }
            } else {
                zValueError(option_mode);
            }

            if(data.isEmpty()) {
                zError << QObject::tr("weekly content is empty.");

                parser.showHelp(-1);
            }
        }

        Tower weekly;

        weekly.init(parser.value(option_email).toUtf8(), parser.value(option_pass).toUtf8(),
                    parser.value(option_date), parser.value(option_filter), parser.isSet(option_default));
        weekly.commitWeekly(data);

        break;
    }
    case Tower::WorkOvertimeType:{
        Tower weekly;

        weekly.init(parser.value(option_email).toUtf8(), parser.value(option_pass).toUtf8(),
                    parser.value(option_date), parser.value(option_filter), parser.isSet(option_default));
        weekly.commitOvertime(parser.value(option_title).toUtf8(), arguments,
                              parser.value(option_start_time).toUtf8(),
                              parser.value(option_end_time).toUtf8());
        break;
    }
    default:
        zErrorQuit;
        break;
    }

    return a.exec();
}

