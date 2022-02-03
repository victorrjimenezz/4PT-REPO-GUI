//
// Created by Víctor Jiménez Rugama on 1/30/22.
//
#ifndef INC_4PT_REPO_GUI_MAINVIEW_H
#define INC_4PT_REPO_GUI_MAINVIEW_H

#include <QApplication>
#include <QPushButton>
#include <QtCore/qfile.h>
#include <QVBoxLayout>
#include <QtUiTools/QUiLoader>
#include <QObject>
#include <QGraphicsView>
#include <QTextEdit>
#include <QListView>
#include <QToolButton>
#include <QFileDialog>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QProgressBar>
#include <QProgressDialog>
#include <QShortcut>
#include <QEvent>
#include <QDropEvent>
#include <QMimeData>

#include <list>
#include <yaml-cpp/yaml.h>
#include "utils.h"
#include "Server.h"
#include "PKGUtils/PKGInfo.h"

#define fileSeparator '/'
#define BufferSize 1024

class mainView : public QWidget {

Q_OBJECT

private slots:
    void exportZip();
    void exportFolder();
    void remove();

    void runServer();

    void selectRepoIcon();

    void addPKG();
    void addFolder();
    void changeType();
    void repoNameChanged();
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
private:
    QWidget widget;
    QGraphicsView * iconQGraphicsView;

    YAML::Node repoYML;
    std::string IPAddress;

    QMenu * typeSelectionMenu;
    QAction * typeMenuActions[APP_TYPES];

    Server * server;

    QGraphicsView * repoIconQGraphicsView;
    uint8_t * repoIcon;
    uint32_t repoIconSize;
    QPushButton* selectRepoIconButton;

    QFileDialog * fileSelector;
    QTextEdit * title;
    QTextEdit * titleID;
    QTextEdit * Version;

    QTextEdit * serverStatus;
    QTextEdit * REPOName;

    QPixmap defaultIcon;
    QListView * fileListView;
    QStandardItemModel fileListModel;

    QPushButton* exportZipButton;
    QPushButton* exportFolderButton;
    QPushButton* removeButton;

    QPushButton* runServerButton;

    QPushButton* addPKGButton;
    QPushButton* addFolderButton;

    QPushButton* typeButton;

    std::list<PKGInfo *> packages;
    std::string progressText;
    int progressCount;
    QShortcut qShortcut;
    QShortcut qShortcutBack;

    void addPackageToList(std::string title);

    void filesSelected();

    void foldersSelected();

    void iterateDir(std::string dir);

    static bool ends_with(std::string const & value, std::string const & ending);

    void loadPKG(const std::string& path);

    void selectionChange(int index);

    void clearView();

    bool isPKG(std::string const & path);
    void copyFiles(std::string file, bool * finished, QProgressDialog * progress);
    void exportFiles(std::string file, bool * finished, QProgressDialog * progress);


public:
    static mainView * mainViewPtr;
    explicit mainView(QWidget *widget);
    uint8_t * getRepoIcon(uint32_t * size);
    std::string getFileLocalPath(const char * ID);
    ~mainView() override;
};
#endif //INC_4PT_REPO_GUI_MAINVIEW_H
