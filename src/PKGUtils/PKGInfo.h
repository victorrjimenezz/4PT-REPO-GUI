//
// Created by Víctor Jiménez Rugama on 1/14/22.
//

#ifndef APT_PKGINFO_H
#define APT_PKGINFO_H

#include <cstdlib>
#include <string>
#include "PKGData.h"
static const char* TypeStr[APP_TYPES]={"GAME", "APP", "UPDATE", "THEME", "TOOL", "CHEAT", "MISC"};

class SFO;
class PKGInfo {
public:
    enum PKGTypeENUM{
        GAME,APP, UPDATE, THEME, TOOL,CHEAT, MISC
    };
private:
    static constexpr int PARAM_SFO_ID = 0x1000;
    static constexpr int PARAM_ICON0_ID = 0x1200;

    enum PKGTypeENUM packageType;
    uint64_t pkgSize;
    uint32_t iconSize;
    uint8_t * icon;
    SFO * sfoFile;
    std::string ID;
    std::string TITLE_ID;
    std::string TITLE;
    std::string path;
    std::string VERSIONSTR;
    std::string PKG_SFO_TYPE;
    double PKG_APP_VERSION = -1;
    double SYSTEM_VERSION = -1;
    void loadData(uint8_t * data, uint64_t data_size);
    uint8_t * loadTable(uint64_t tableOffset, uint64_t tableSize);
    uint8_t * loadSFO(uint64_t param_sfo_offset, uint64_t param_sfo_size);
    uint8_t * loadIcon(uint64_t icon0_png_offset, uint64_t icon0_size);
    std::string genRandom(int len);
public:
    explicit PKGInfo(const std::string& fromPath);
    explicit PKGInfo(uint8_t * data, uint64_t data_size);
    std::string getTitleID();
    std::string getTitle();
    std::string getPath();
    std::string getType();
    std::string getID();
    uint64_t getPkgSize();
    uint8_t * getIcon();
    uint32_t getIconSize();
    double getVersion() const;
    double getSystemVersion() const;
    ~PKGInfo();

    std::string getVersionString();

    void changeType(int type);
};
#endif //APT_PKGINFO_H
