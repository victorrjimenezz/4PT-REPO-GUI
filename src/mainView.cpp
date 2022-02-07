
// Created by Víctor Jiménez Rugama on 1/30/22.
//
#include "mainView.h"
#include <iostream>
#include <ostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <queue>
#include <thread>
#include "sandbird/sandbird.h"
#include "Server.h"
#include "zip/zip.h"
#ifdef _WIN32
#define appDefaultIcon "appDefaultIcon.png"
#elif __APPLE__
#define appDefaultIcon ":/images/appDefaultIcon.png"
#endif

mainView * mainView::mainViewPtr = nullptr;

const auto copyOptions = std::filesystem::copy_options::overwrite_existing;
mainView::mainView(QWidget *widget) : widget(widget), fileListModel(), defaultIcon(appDefaultIcon), repoYML(), qShortcut(Qt::Key_Delete, widget),qShortcutBack(Qt::Key_Backspace, widget), packages() {
    for(auto & package : packages)
        package = nullptr;
    fileSelector = new QFileDialog();

    repoIcon = nullptr;
    repoIconSize = 0;

    typeSelectionMenu = new QMenu();
    QAction * action;
    for(int i =0; i<APP_TYPES;i++) {
        action = new QAction(tr(TypeStr[i]));
        action->setObjectName(tr(TypeStr[i]));
        action->setProperty("typeVal",i);
        connect(action,SIGNAL(triggered()),this,SLOT(changeType()));
        action->setText(tr(TypeStr[i]));
        typeMenuActions[i] = action;
        typeSelectionMenu->addAction(action);
    }

    exportZipButton = widget->findChild<QPushButton*>("EXPORTZIP");
    exportFolderButton = widget->findChild<QPushButton*>("EXPORTFOLDER");
    removeButton = widget->findChild<QPushButton*>("REMOVE");

    runServerButton = widget->findChild<QPushButton*>("RUNSERVER");

    addPKGButton = widget->findChild<QPushButton*>("ADD_PKG");
    addFolderButton = widget->findChild<QPushButton*>("ADD_FOLDER");

    typeButton = widget->findChild<QPushButton*>("TYPE");

    selectRepoIconButton = widget->findChild<QPushButton*>("SELECTICON");

    iconQGraphicsView = widget->findChild<QGraphicsView*>("ICON");
    repoIconQGraphicsView = widget->findChild<QGraphicsView*>("REPO_ICON");

    title = widget->findChild<QTextEdit*>("TITLE");
    titleID = widget->findChild<QTextEdit*>("TITLE_ID");
    Version = widget->findChild<QTextEdit*>("VERSION");

    serverStatus = widget->findChild<QTextEdit*>("SERVER_STATUS");
    REPOName = widget->findChild<QTextEdit*>("REPO_NAME");

    fileListView = widget->findChild<QListView*>("FILELIST");
    fileListView->setModel(&fileListModel);

    //fileListView->setSpacing(5);
    qShortcut.setAutoRepeat(false);
    qShortcutBack.setAutoRepeat(false);

    connect(exportZipButton, SIGNAL(clicked()), this, SLOT(exportZip()));
    connect(exportFolderButton, SIGNAL(clicked()), this, SLOT(exportFolder()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));

    connect(runServerButton, SIGNAL(clicked()), this, SLOT(runServer()));
    connect(REPOName, SIGNAL(textChanged()), this, SLOT(repoNameChanged()));

    connect(selectRepoIconButton, SIGNAL(clicked()), this, SLOT(selectRepoIcon()));

    connect(addPKGButton, SIGNAL(clicked()), this, SLOT(addPKG()));
    connect(addFolderButton, SIGNAL(clicked()), this, SLOT(addFolder()));

    connect(&qShortcut, SIGNAL(activated()), this, SLOT(remove()));
    connect(&qShortcutBack, SIGNAL(activated()), this, SLOT(remove()));

    fileListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    fileListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(fileListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &mainView::selectionChanged);

    fileListView->installEventFilter(this);

    repoIconQGraphicsView->setScene(new QGraphicsScene);
    repoIconQGraphicsView->scene()->setSceneRect(repoIconQGraphicsView->rect());
    repoIconQGraphicsView->setEnabled(false);

    iconQGraphicsView->setScene(new QGraphicsScene);
    iconQGraphicsView->scene()->setSceneRect(iconQGraphicsView->rect());
    iconQGraphicsView->setEnabled(false);

    typeButton->setMenu(typeSelectionMenu);

    server = Server::initServer(&repoYML);
    serverStatus->setText(tr("Server Currently Offline"));
    runServerButton->setText(tr("Start Server"));

    IPAddress = getIP();
    mainViewPtr = this;

    repoYML["iconPath"] = repoIconName;
    repoYML["name"] = "Default Repo Name";

}

bool mainView::ends_with(std::string const & value, std::string const & ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
void mainView::exportFiles(std::string file, bool * finished, QProgressDialog * progress){
    std::stringstream repoYAMLString;
    std::string ymlStr, packageName;
    FILE * pkg;
    char buff[BufferSize];

    auto * zip = zip_open(file.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');

    if(progress->wasCanceled())
        goto canceled;

    progressCount++;

    if(repoIcon != nullptr && repoIconSize >0) {
        progressText = "Copying icon.png";
        zip_entry_open(zip,"icon.png");
        zip_entry_write(zip, repoIcon, repoIconSize);
        zip_entry_close(zip);
    }

    if(progress->wasCanceled())
        goto canceled;
    progressText = "Copying repo.yml";
    progressCount++;
    repoYAMLString << repoYML;
    ymlStr = repoYAMLString.str();


    zip_entry_open(zip,"repo.yml");
    zip_entry_write(zip, ymlStr.c_str(), ymlStr.size());
    zip_entry_close(zip);

    zip_entry_open(zip,"pkg/");
    zip_entry_close(zip);

    for(PKGInfo * package : packages){

        if(progress->wasCanceled())
            goto canceled;
        progressText = "Copying " + package->getTitle();
        progressCount++;

        packageName = "pkg/";
        packageName += package->getID()+".pkg";
        pkg = fopen(package->getPath().c_str(),"rb");
        if(pkg == nullptr)
            continue;
        fseek(pkg, 0L, SEEK_END);
        size_t sz = ftell(pkg);
        size_t bytesRead = 0;
        size_t currBytes = BufferSize;
        rewind(pkg);
        zip_entry_open(zip,packageName.c_str());

        while(bytesRead < sz) {
            if(currBytes+ bytesRead > sz)
                currBytes = sz - bytesRead;

            fread(&buff[0],1,currBytes,pkg);
            bytesRead+= currBytes;

            zip_entry_write(zip, &buff[0],currBytes);
        }
        fclose(pkg);
        zip_entry_close(zip);
    }

    zip_close(zip);
    goto fini;
    return;
    canceled:
    if(std::filesystem::exists(file))
        std::filesystem::remove(file);
    fini:
    *finished = true;

}
void mainView::exportZip() {

    auto repoName = repoYML["name"].as<std::string>("");
    int icon;
    static const char *exportFileName;
    bool finished = false;


    if(repoName.empty()) {
        repoName = "Default Repo Name";
        repoYML["name"] = repoName;
    }
    repoName += ".zip";
    std::string fileName = QFileDialog::getSaveFileName(fileSelector, tr("Save File"),QString::fromStdString(repoName), tr("ZIP files (*.zip)")).toStdString();

    QCoreApplication::processEvents();
    fileSelector->hide();
    icon = 1+(repoIcon != nullptr && repoIconSize >0);
    QProgressDialog progress("Exporting files...", "Abort Export", 0, packages.size()+icon, &widget);
    progress.setWindowTitle("Progress");
    progress.setWindowModality(Qt::WindowModal);
    progress.setAttribute(Qt::WA_ShowWithoutActivating);
    progress.setWindowFlag(Qt::WindowDoesNotAcceptFocus, true);
    progress.setValue(0);
    progress.setVisible(true);
    progress.open();

    if(fileName.empty())
        goto canceled;
    if(std::filesystem::exists(fileName))
        std::filesystem::remove(fileName);

    exportFileName = fileName.c_str();
    fileName = std::filesystem::relative(fileName,std::filesystem::current_path()).string();
    std::thread(&mainView::exportFiles,this,fileName,&finished,&progress).detach();

    while(!finished) {
        progress.setLabelText(progressText.c_str());
        progress.setValue(progressCount);
        QCoreApplication::processEvents();
    }

    canceled:
    progress.close();
    progress.setVisible(false);
}
void mainView::copyFiles(std::string file, bool * finished, QProgressDialog * progress) {
    std::string saveInDir = file+fileSeparator;
    std::string saveInDirPKG = saveInDir+PKGFolder+fileSeparator;
    std::string repoYMLPath = saveInDir+"repo.yml";
    std::ofstream saveYML;
    std::string repoIconPath = saveInDir+repoIconName;
    std::string pkgDestPath;
    std::string pkgPath;
    if(!std::filesystem::exists(saveInDir))
        std::filesystem::create_directory(saveInDir);
    if(!std::filesystem::exists(saveInDirPKG))
        std::filesystem::create_directory(saveInDirPKG);
    for(PKGInfo * pkg : packages){
        if(progress->wasCanceled())
            goto canceled;
        progressCount++;
        progressText = ("Copying "+pkg->getTitle());
        pkgPath = pkg->getPath();
        pkgDestPath = saveInDirPKG+pkg->getID()+".pkg";
        std::filesystem::copy(pkgPath,pkgDestPath,copyOptions);

    }


    if(progress->wasCanceled())
        goto canceled;
    progressCount++;
    progressText = "Copying repo.yml";
    saveYML.open(repoYMLPath, std::ofstream::out | std::ofstream::trunc);
    if(saveYML.fail())
        goto canceled;
    saveYML << repoYML;
    saveYML.flush();
    saveYML.close();

    if(repoIconSize>0) {
        progressText =  "Copying icon";
        std::ofstream repoIconFile(repoIconPath, std::ios::binary);
        if (repoIconFile.fail())
            goto canceled;
        for (int i = 0; i < repoIconSize; i++)
            repoIconFile << repoIcon[i];

        repoIconFile.flush();
        repoIconFile.close();
    }

    if(progress->wasCanceled())
        goto canceled;
    progressCount++;

    canceled:
    *finished = true;
}
void mainView::exportFolder() {
    int icon = 1+(repoIcon != nullptr && repoIconSize >0);
    progressText = "Copying files...";
    QProgressDialog progress(progressText.c_str(), "Abort Copy", 0, packages.size()+icon, &widget);
    progress.setWindowTitle("Progress");

    fileSelector->setFileMode(QFileDialog::Directory);
    fileSelector->exec();
    if(fileSelector->selectedFiles().empty())
        return;
    YAML::Node file;

    progressCount = 0;
    fileSelector->hide();
    progress.setWindowModality(Qt::WindowModal);
    progress.setAttribute(Qt::WA_ShowWithoutActivating);
    progress.setWindowFlag(Qt::WindowDoesNotAcceptFocus, true);
    progress.setValue(progressCount);
    progress.setVisible(true);
    QCoreApplication::processEvents();
    progress.open();
    bool finished = false;
    for(const QString& selectedFile :fileSelector->selectedFiles()) {
        std::thread(&mainView::copyFiles,this,selectedFile.toStdString(),&finished,&progress).detach();
        //std::thread(&mainView::copyFiles,selectedFile.toStdString(),std::ref(finished),std::ref(progress),this).detach();
        break;
    }
    while(!finished) {
        progress.setLabelText(progressText.c_str());
        progress.setValue(progressCount);
        QCoreApplication::processEvents();
    }
    progress.setVisible(false);
    progress.close();
    std::cout << "finished" << std::endl;
    progress.hide();
}
void mainView::remove() {
    QModelIndexList indexes = fileListView->selectionModel()->selectedIndexes();
    PKGInfo * currPKGPtr;
    if(indexes.empty())
        return;
    int index = indexes.at(0).row();

    if(index > packages.size())
        return;


    auto listFront = packages.begin();
    std::advance(listFront, index);

    currPKGPtr = *listFront;

    if(index > 0 && index< packages.size())
        fileListView->setCurrentIndex(fileListView->model()->index(index-1,0));
    else
        fileListView->setCurrentIndex(fileListView->model()->index(0,0));

    fileListModel.removeRow(index);
    packages.erase(listFront);
    clearView();
    if(index==0)
        selectionChange(index);
    else
        selectionChange(index-1);
    repoYML.remove(currPKGPtr->getID());
    delete currPKGPtr;
}

void mainView::runServer() {
    if(server->running()) {
        runServerButton->setDisabled(true);
        serverStatus->setText(tr("Stopping Server"));
        std::thread(&mainView::stopServer,this).detach();
    } else if(server->startServer()){
        runServerButton->setText(tr("Stop Server"));
        std::string IPString = "Server Running on ";
        IPString+= IPAddress;
        serverStatus->setText(tr(IPString.c_str()));
    } else {
        std::string IPString = "Could not start server on ";
        IPString+= IPAddress;
        serverStatus->setText(tr(IPString.c_str()));
    }
}
void mainView::addPKG() {
    QString pkgFilter = tr("PKG (*.pkg)");
    fileSelector->setFileMode(QFileDialog::ExistingFiles);
    fileSelector->setNameFilter(pkgFilter);
    fileSelector->exec();
    if(fileSelector->selectedFiles().empty())
        return;
    filesSelected();
}
void mainView::addFolder() {
    fileSelector->setFileMode(QFileDialog::Directory);
    fileSelector->exec();
    if(fileSelector->selectedFiles().empty())
        return;
    foldersSelected();
}

void mainView::filesSelected() {
    for(const QString& selectedFile :fileSelector->selectedFiles())
        loadPKG(selectedFile.toStdString());
}

void mainView::foldersSelected() {
    for(const QString& selectedFolder :fileSelector->selectedFiles()){
        iterateDir(selectedFolder.toStdString());
    }
}

mainView::~mainView() {

    QList<QAction *> actions = typeSelectionMenu->actions();

    for(int i = 0; i < actions.size(); i++) {
        //actions.remove(i);
        delete actions[i];
    }

    Server::termServer();
    delete iconQGraphicsView->scene();
    delete repoIconQGraphicsView->scene();
    delete typeSelectionMenu;
    delete fileSelector;
}

void mainView::iterateDir(std::string directory) {
    std::queue<std::string> pendingDirs;
    pendingDirs.push(directory);

    while(!pendingDirs.empty()) {
        std::string currentDir = std::string(pendingDirs.front());
        pendingDirs.pop();
        for (const auto &entry: std::filesystem::directory_iterator(currentDir)) {
            std::string path = std::string(entry.path().string());
            if (entry.is_directory())
                pendingDirs.push(path);
            else if(ends_with(path,".pkg"))
                loadPKG(path);

        }
    }
}

void mainView::loadPKG(const std::string& path) {
    auto * package = new PKGInfo(path);
    std::string pkgPath = PKGFolder;
    if(std::equal(package->getTitle().begin(), package->getTitle().end(),"") || std::equal(package->getTitleID().begin(), package->getTitleID().end(),"") || package->getVersion()==-1){
        std::cout << "Invalid PKG at " << path << std::endl;
        goto err;
    }
    packages.emplace_back(package);
    addPackageToList(package->getTitle());
    repoYML[package->getID()];
    pkgPath += fileSeparator;
    pkgPath += package->getID();
    pkgPath+= ".pkg";
    repoYML[package->getID()]["pkgPath"] =pkgPath;
    repoYML[package->getID()]["type"] = package->getType();
    return;
    err:
    delete package;
}

void mainView::addPackageToList(std::string title) {
    auto * item = new QStandardItem(QString::fromStdString(title));
    fileListModel.appendRow(item);
}

void mainView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    auto indexes = selected.indexes();
    if(indexes.empty())
        return;
    int index = indexes.at(0).row();
    selectionChange(index);
}
void mainView::selectionChange(int index) {
    PKGInfo * currentPackagePtr;
    if(index > packages.size())
        return;

    auto listFront = packages.begin();
    std::advance(listFront, index);
    currentPackagePtr = *listFront;

    if(currentPackagePtr == nullptr)
        return;
    title->setText(QString::fromStdString(currentPackagePtr->getTitle()));
    titleID->setText(QString::fromStdString(currentPackagePtr->getTitleID()));
    Version->setText(QString::fromStdString(currentPackagePtr->getVersionString()));
    typeButton->setText(QString::fromStdString(currentPackagePtr->getType()));

    QList<QGraphicsItem *> qItems = iconQGraphicsView->scene()->items();
    for(auto item : qItems){
        iconQGraphicsView->scene()->removeItem(item);
        delete item;
    }
    if(currentPackagePtr->getIcon() != nullptr && currentPackagePtr->getIconSize() >0){
        QPixmap pix;
        pix.loadFromData(currentPackagePtr->getIcon(),currentPackagePtr->getIconSize());
        iconQGraphicsView->scene()->addPixmap(pix.scaled(iconQGraphicsView->size(),Qt::KeepAspectRatio));
    } else{
        iconQGraphicsView->scene()->addPixmap(defaultIcon.scaled(iconQGraphicsView->size(),Qt::KeepAspectRatio));
    }

}

void mainView::changeType() {
    QModelIndexList indexes = fileListView->selectionModel()->selectedIndexes();
    if(indexes.empty())
        return;
    int index = indexes.at(0).row();
    if(index >= packages.size())
        return;
    auto *act = qobject_cast<QAction*>(sender());
    if(act) {
        // look, no pointers!
        auto listFront = packages.begin();
        std::advance(listFront, index);
        PKGInfo * currPKGPtr = *listFront;
        if(currPKGPtr== nullptr)
            return;
        int type = act->property("typeVal").toInt();
        currPKGPtr->changeType(type);
        typeButton->setText(QString::fromStdString(currPKGPtr->getType()));
        repoYML[currPKGPtr->getID()]["type"] = currPKGPtr->getType();
    }

}

void mainView::clearView() {

    QList<QGraphicsItem *> items = iconQGraphicsView->scene()->items();
    for (auto gi : items){
        if(gi->parentItem()==NULL) {
            delete gi;
        }
    }

    title->setText(QString::fromStdString(""));
    titleID->setText(QString::fromStdString(""));
    Version->setText(QString::fromStdString(""));
    typeButton->setText(QString::fromStdString(TypeStr[PKGInfo::PKGTypeENUM::MISC]));

    QPixmap pix;
    iconQGraphicsView->scene()->addPixmap(pix);

}

void mainView::selectRepoIcon() {
    QString imgFilter = tr("Images (*.png *.gif *.jpg *.jpeg *.bmp *.ico *.tga *.tif *.tiff)");
    fileSelector->setFileMode(QFileDialog::ExistingFile);
    fileSelector->setNameFilter(imgFilter);
    fileSelector->exec();
    if(fileSelector->selectedFiles().empty())
        return;
    for(const QString& selectedFile :fileSelector->selectedFiles()) {
        std::ifstream input(selectedFile.toStdString(), std::ios::binary);
        std::vector<uint8_t> bytes(
                (std::istreambuf_iterator<char>(input)),
                (std::istreambuf_iterator<char>()));
        input.close();
        uint32_t byteSize = bytes.size();

        if(byteSize==0)
            return;
        repoIconSize = 0;
        if(repoIcon!= nullptr) {
            free(repoIcon);
            repoIcon = nullptr;
        }

        QList<QGraphicsItem *> items = repoIconQGraphicsView->scene()->items();
        for (auto gi : items){
            if(gi->parentItem()==NULL) {
                delete gi;
            }
        }

        repoIcon = (uint8_t *) malloc(sizeof(uint8_t) * byteSize);
        for(int i =0; i< byteSize; i++)
            repoIcon[i] = bytes[i];
        repoIconSize = byteSize;
        QPixmap pix;
        pix.loadFromData(repoIcon,repoIconSize);
        repoIconQGraphicsView->scene()->addPixmap(pix.scaled(repoIconQGraphicsView->size(),Qt::KeepAspectRatio));

    }
}

void mainView::repoNameChanged() {
    repoYML["name"] = REPOName->toPlainText().toStdString();
}

uint8_t *mainView::getRepoIcon(uint32_t *size) {
    *size = repoIconSize;
    return repoIcon;
}

std::string mainView::getFileLocalPath(const char *ID) {
    for(PKGInfo * pkgInfo : packages)
        if(strcasecmp(ID,pkgInfo->getID().c_str()) == 0)
            return pkgInfo->getPath();
    return "";
}

bool mainView::eventFilter(QObject *obj, QEvent *ev) {
    if(ev->type()==QEvent::DragEnter) {
        auto* dragEnterEvent = dynamic_cast<QDragEnterEvent*>(ev);
        bool accept = dragEnterEvent->mimeData()->hasUrls();
        ev->setAccepted(accept);
        return accept;
    } else if(ev->type()==QEvent::Drop){
        auto* dropEvent = dynamic_cast<QDropEvent*>(ev);
        auto * mimeData = dropEvent->mimeData();
        if(!mimeData->hasUrls())
            goto end;
        std::string pkgPath;
        for(const auto& url : mimeData->urls()) {
           pkgPath = url.path().toStdString();
#ifdef _WIN32
           //Remove first '/' Character in windows
            pkgPath = pkgPath.substr(1);
#endif
           if(isPKG(pkgPath)){
               loadPKG(pkgPath);
           } else if (std::filesystem::is_directory(pkgPath)){
               iterateDir(pkgPath);
           }
        }
    }
    end:
    return false;
}

bool mainView::isPKG(const std::string &path) {
    return ends_with(path,".pkg") && std::filesystem::exists(path);
}
void mainView::stopServer() {
    if(server->stopServer())
        QMetaObject::invokeMethod(this,"serverStopped",Qt::AutoConnection);

}
void mainView::serverStopped() {
    runServerButton->setText(tr("Start Server"));
    serverStatus->setText(tr("Server Currently Offline"));
    runServerButton->setDisabled(false);
}