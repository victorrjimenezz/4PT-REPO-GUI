//
// Created by Víctor Jiménez Rugama on 1/31/22.
//
#include "Server.h"
#include "mainView.h"
#include <thread>
#include <filesystem>

std::mutex mtx;
Server * Server::mainServer = nullptr;

int Server::event_handler(sb_Event *e) {
    return mainServer->handleEvent(e);
}
Server * Server::initServer(YAML::Node * YAML){
    if(mainServer!= nullptr)
        return nullptr;
    return new Server(YAML);
}
Server::Server(YAML::Node * YAML) {
    mainServer = this;
    srv = nullptr;
    this->YAML = YAML;
    packagePath = '/';
    packagePath += PKGFolder;
    packagePath = '/';
}
Server::~Server() {
    stopServer();
}
bool Server::startServer() {
    if(srv!= nullptr)
        return false;
    sb_Options opt;

    memset(&opt, 0, sizeof(opt));
#ifdef _WIN32
    opt.host = getIP().c_str();
#endif
    opt.port = "80";
    opt.handler = event_handler;

    srv = sb_new_server(&opt);
    std::thread(&Server::poll,std::ref(*this)).detach();
    return srv != nullptr;

}
bool Server::stopServer() {
    if(srv== nullptr)
        return false;
    sb_Server *oldSrv = srv;
    srv = nullptr;
    sb_close_server(oldSrv);
    return srv == nullptr;

}
bool Server::running() {
    return srv != nullptr;
}

void Server::poll() {
    mtx.lock();
    while(srv != nullptr)
        sb_poll_server(srv, 1000);
    mtx.unlock();
}

int Server::handleEvent(sb_Event *e) {
    bool isPackage = false;
    std::string ID;
    bool isRepository = false;
    bool isRepoIcon = false;
    if (e->type == SB_EV_REQUEST) {
        ID = isPKG(e->path);
        isPackage = !ID.empty();
        isRepository = isRepo(e->path);
        isRepoIcon = isIcon(e->path);
        if(!isPackage && !isRepository && !isRepoIcon) {
            goto fail;
        }
        //sb_send_status(e->stream, 200, "OK");
        //sb_send_header(e->stream, "Content-Type", "text/plain");
        if(isRepository) {
            if(!sendYML(e)){
                goto fail;
            }
        }else if(isPackage){
            if(!sendPKG(ID.c_str(),e)) {
                goto fail;
            }
        } else if(isRepoIcon){
            if(!sendIcon(e)){
                goto fail;
            }
        }
    }
    return SB_RES_OK;
    fail:
    sb_send_status(e->stream, 404, "NOT FOUND");
    return SB_RES_CLOSE;
}

bool Server::termServer() {
    delete mainServer;
    mainServer = nullptr;
    return true;
}

bool Server::ends_with(std::string const & value, std::string const & ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
std::string Server::isPKG(const char *path) {
    std::string pathStr = path;
    std::string TITLEID;
    if(pathStr.size()>= packagePath.size() && packagePath.find_first_of(pathStr) == 0 && std::count(pathStr.begin(), pathStr.end(), '/') == 2 && ends_with(pathStr,".pkg")) {
        TITLEID = pathStr.substr(pathStr.find_last_of('/') + 1);
        TITLEID.pop_back();
        TITLEID.pop_back();
        TITLEID.pop_back();
        TITLEID.pop_back();
    }
    return TITLEID;
}

bool Server::isRepo(const char *path) {
    return strcasecmp(path,"/repo.yml") == 0;
}

bool Server::isIcon(const char *path) {
    std::string repoIcon = "/";
    repoIcon += repoIconName;
    return strcasecmp(path,repoIcon.c_str()) == 0;
}
bool Server::sendPKG(const char * ID, sb_Event *e){
    count ++;
    bool hasRange;
    std::string filePath;
    if(!(*YAML)[ID])
        goto fail;

    if(!(*YAML)[ID]["pkgPath"])
        goto fail;

    filePath = mainView::mainViewPtr->getFileLocalPath(ID);
    if(filePath.empty() || !std::filesystem::exists(filePath))
        goto fail;

    char header[50];
    hasRange = !sb_get_header(e->stream,"Range",&header[0],50);

    if(hasRange) {
        std::string headerStr = header;
        headerStr = headerStr.substr(headerStr.find_first_of('=')+1);
        long firstByte = std::stol(headerStr.substr(0,headerStr.find_first_of('-')));

        std::string lastByteStr = headerStr.substr(headerStr.find_first_of('-')+1);
        if (firstByte >= std::filesystem::file_size(filePath))
            goto fail;
        if(lastByteStr.empty()){
            sb_send_status(e->stream, 206, "OK");
            sb_send_file_from_byte(e->stream,filePath.c_str(),firstByte);
        } else {
            long lastByte = std::stol(lastByteStr);
            if (firstByte > lastByte || std::filesystem::file_size(filePath) <= lastByte)
                goto fail;
            FILE *file = fopen(filePath.c_str(), "rb");
            if (file == nullptr)
                goto fail;
            uint64_t readSize = lastByte - firstByte;
            fseek(file, firstByte, SEEK_CUR);
            auto *data = (uint8_t *) malloc(sizeof(uint8_t) * readSize);
            fread(&data[0], 1, readSize, file);
            sb_send_status(e->stream, 206, "OK");
            sb_send_bytes(e->stream, data, readSize);
            free(data);
            fclose(file);
        }
    } else {
        sb_send_status(e->stream, 200, "OK");
        sb_send_file(e->stream,filePath.c_str());
    }

    return true;
    fail:
    return false;
}

bool Server::sendIcon(sb_Event *e) {
    uint32_t iconSize;
    uint8_t * icon;
    int err;
    icon = mainView::mainViewPtr->getRepoIcon(&iconSize);
    if(iconSize == 0 || icon == nullptr){
        goto fail;
    }

    sb_send_bytes(e->stream,icon,iconSize);

    return true;
    fail:
    return false;

}

bool Server::sendYML(sb_Event *e) {
    std::stringstream ss;
    ss << *YAML;
    std::string YMLStr = ss.str();
    const char * YAMLString = YMLStr.c_str();
    uint32_t strlen = YMLStr.size();
    //SB_SEND_BYTES takes care of deallocating the memory
    sb_send_bytes(e->stream, (uint8_t *) &YAMLString[0], strlen);
    return true;
}

