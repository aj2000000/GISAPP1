#include <QApplication>
#include <QDebug>
#include "MainApplication.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainApplication mainApp;
    if (!mainApp.initialize()) {
        qCritical() << "Application initialization failed. Exiting.";
        return -1;
    }

    return app.exec();
}

