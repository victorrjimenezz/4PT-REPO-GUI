//
// Created by Víctor Jiménez Rugama on 1/31/22.
//

#ifndef INC_4PT_REPO_GUI_SERVER_H
#define INC_4PT_REPO_GUI_SERVER_H
#include "sandbird/sandbird.h"
#include "PKGUtils/PKGData.h"
#include <yaml-cpp/yaml.h>
#include <mutex>

class Server {
private:
    static std::mutex srvMtx;
    int count =0;
    std::string packagePath;
    sb_Server *srv;
    YAML::Node * YAML;
    static int event_handler(sb_Event *e);
    int handleEvent(sb_Event *e);
    explicit Server(YAML::Node * YAML);
    bool isRepo(const char * path);
    bool isIcon(const char * path);
    std::string isPKG(const char * path);
    bool sendIcon(sb_Event * e);
    bool sendYML(sb_Event * e);
    bool sendPKG(const char * ID, sb_Event * e);
    static bool ends_with(std::string const & value, std::string const & ending);
    void poll();
public:
    static Server * mainServer;
    static Server * initServer(YAML::Node * YAML);
    static bool termServer();
    bool startServer();
    bool stopServer();
    bool running();
    ~Server();
};
#endif //INC_4PT_REPO_GUI_SERVER_H
