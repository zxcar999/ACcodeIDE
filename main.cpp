#include "accodeide.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <ui_accodeide.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ACcodeIDE_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    ACcodeIDE w;
    w.show();
    return a.exec();
}
