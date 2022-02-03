#include <QApplication>
#include <QPushButton>
#include <QtCore/qfile.h>
#include <QtUiTools/QUiLoader>
#ifdef _WIN32
//Include QtGui to free console
#include <QtGui>
#elif __APPLE__
#include <string>
#include <filesystem>
#endif

#include "mainView.h"


int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication a(argc, argv);
#ifdef __APPLE__
    //Set current path to resources
    std::string currPath = a.applicationDirPath().toStdString();
    currPath = currPath.substr(0,currPath.find_last_of('/')) + "/Resources/";
    std::filesystem::current_path(currPath);
#elif _WIN32
    //Free Console
    FreeConsole();
#endif
    QFile file("mainView.ui");
    file.open(QFile::ReadOnly);

    QUiLoader loader = QUiLoader();
    QWidget * window = loader.load(&file);

    file.close();
    window->show();
    mainView mv(window);

    return QApplication::exec();
}