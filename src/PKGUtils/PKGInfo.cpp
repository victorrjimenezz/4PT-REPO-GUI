//
// Created by Víctor Jiménez Rugama on 1/14/22.
//
#include "PKGInfo.h"
#include "PKGData.h"
#include "SFO.h"
#include <string>
#include <filesystem>
#include <sstream>

PKGInfo::PKGInfo(const std::string& fromPath) {
    path = fromPath;
    ID = "";
    TITLE_ID = "";
    TITLE = "";
    VERSIONSTR = "";
    packageType = MISC;
    PKG_APP_VERSION = -1;
    iconSize = 0;
    icon = nullptr;
    sfoFile = nullptr;
    if(std::filesystem::file_size(fromPath) < PKG_INITIAL_BUFFER_SIZE)
        return;
    auto * data = (uint8_t *) malloc(PKG_INITIAL_BUFFER_SIZE);

        FILE * file = fopen(fromPath.c_str(), "rb");
        if(file == nullptr) {
            fclose(file);
            goto err;
        }
        fread(data,PKG_INITIAL_BUFFER_SIZE,1,file);
        fclose(file);
    loadData(data, PKG_INITIAL_BUFFER_SIZE);
    ID = TITLE_ID+genRandom(7);
    err:
    free(data);
}
void PKGInfo::loadData(uint8_t * data, uint64_t data_size){
    bool neededMargin = false;
    uint32_t entrycountRAW;
    memcpy(&entrycountRAW, &data[PKG_ENTRY_COUNT], sizeof(uint32_t));
    size_t entry_count = BE32(entrycountRAW);

    uint32_t tableOffsetRAW;
    memcpy(&tableOffsetRAW, &data[PKG_ENTRY_TABLE_OFFSET], sizeof(uint32_t));
    uint32_t tableOffset = BE32(tableOffsetRAW);

    uint64_t tableSize = entry_count * SIZEOF_PKG_TABLE_ENTRY;

    memcpy(&pkgSize, &data[PKG_SIZE_OFFSET], sizeof(uint64_t));
    pkgSize = BE64(pkgSize);

    uint32_t param_sfo_offset = 0;
    uint32_t param_sfo_size = 0;

    uint32_t icon0_png_offset = 0;
    uint32_t icon_size = 0;

    uint8_t * entry_table_data;
    if(tableOffset+tableSize < data_size) {
        entry_table_data = &data[tableOffset];
    }else {
        entry_table_data = loadTable(tableOffset, tableSize);
        if(entry_table_data == nullptr)
            return;
        neededMargin = true;
    }

    uint64_t currPos = 0x0;
    uint32_t id;
    uint32_t offset;
    uint32_t size;
    for (int i = 0; i < entry_count; ++i) {

        memcpy(&id,&entry_table_data[currPos], sizeof(uint32_t));
        memcpy(&offset,&entry_table_data[currPos+PKG_ENTRY_OFFSET], sizeof(uint32_t));
        memcpy(&size,&entry_table_data[currPos+PKG_ENTRY_SIZE], sizeof(uint32_t));

        switch (BE32(id)) {
            //SFO
            case PARAM_SFO_ID:
                param_sfo_offset = BE32(offset);
                param_sfo_size = BE32(size);
                break;
                //ICON
            case PARAM_ICON0_ID:
                icon0_png_offset = BE32(offset);
                icon_size = BE32(size);
                break;
            default:
                goto next;
        }
        next:
        currPos+=SIZEOF_PKG_TABLE_ENTRY;
    }

    if(param_sfo_offset > 0 && param_sfo_size > 0){
        uint8_t * sfo = loadSFO(param_sfo_offset,param_sfo_size);
        if(sfo != nullptr) {
            sfoFile = new SFO(sfo);
            char * sfoParam = (char*)sfoFile->getEntry("TITLE_ID");
            if(sfoParam != nullptr)
                TITLE_ID = std::string(sfoParam);

            sfoParam = (char*)sfoFile->getEntry("TITLE");
            if(sfoParam != nullptr)
                TITLE = std::string(sfoParam);

            sfoParam = (char*)sfoFile->getEntry("APP_VER");
            std::stringstream ss;
            ss << std::setprecision(2) << std::fixed;
            if(sfoParam != nullptr) {
                PKG_APP_VERSION = std::stod(sfoParam);
                ss << PKG_APP_VERSION;
                VERSIONSTR = ss.str();
            }else {
                sfoParam = (char*)sfoFile->getEntry("VERSION");
                if(sfoParam!= nullptr) {
                    PKG_APP_VERSION = std::stod(sfoParam);
                    ss << PKG_APP_VERSION;
                    VERSIONSTR = ss.str();
                }
            }

            sfoParam = (char*)sfoFile->getEntry("CATEGORY");
            if(sfoParam != nullptr)
                PKG_SFO_TYPE = std::string(sfoParam);

            sfoParam = (char*)sfoFile->getEntry("SYSTEM_VER");
            if(sfoParam!= nullptr) {
                char sysverchar[4];
                snprintf(&sysverchar[0],4,"%0.2X%0.2X",sfoParam[3],sfoParam[2]);
                SYSTEM_VERSION = std::stod(std::string(&sysverchar[0]))/10.;
            }

            free(sfo);
        }
    }

    if(icon0_png_offset > 0 && icon_size > 0)
        icon = loadIcon(icon0_png_offset,icon_size);

    if(neededMargin)
        free(entry_table_data);


}
std::string PKGInfo::getTitleID(){
    return TITLE_ID;
}
std::string PKGInfo::getTitle(){
    return TITLE;
}
std::string PKGInfo::getType(){
    return TypeStr[packageType];
}
std::string PKGInfo::getVersionString(){
    return VERSIONSTR;
}
double PKGInfo::getVersion() const {
    return PKG_APP_VERSION;
}

double PKGInfo::getSystemVersion() const {
    return SYSTEM_VERSION;
}

PKGInfo::~PKGInfo() {
    if(icon!= nullptr)
        free(icon);
    delete sfoFile;
}

uint8_t *PKGInfo::getIcon() {
    return icon;
}

uint8_t *PKGInfo::loadTable(uint64_t tableOffset, uint64_t tableSize) {
    auto * table = (uint8_t *) malloc(tableSize);

        FILE * file = fopen(path.c_str(), "rb");
        if(file == nullptr) {
            fclose(file);
            goto err;
        }
        fseek(file, (long)tableOffset, SEEK_CUR);
        fread(table,tableSize,1,file);
        fclose(file);

    return table;
    err:
    free(table);
    return nullptr;
}

uint8_t *PKGInfo::loadIcon(uint64_t icon0_png_offset, uint64_t icon0_size) {
    uint8_t * iconPtr = (uint8_t *) malloc(icon0_size);

        FILE * file = fopen(path.c_str(), "rb");
        if(file == NULL) {
            fclose(file);
            goto err;
        }
        fseek(file, (long)icon0_png_offset, SEEK_CUR);
        fread(iconPtr,icon0_size,1,file);
        fclose(file);
        iconSize = icon0_size;
    return iconPtr;
    err:
    free(iconPtr);
    iconPtr = nullptr;
    return iconPtr;
}


uint8_t *PKGInfo::loadSFO(uint64_t param_sfo_offset, uint64_t param_sfo_size) {
    auto * SFO = (uint8_t *) malloc(param_sfo_size);

        FILE * file = fopen(path.c_str(), "rb");
        if(file == nullptr) {
            fclose(file);
            goto err;
        }
        fseek(file, (long)param_sfo_offset, SEEK_CUR);
        fread(SFO,param_sfo_size,1,file);
        fclose(file);



    return SFO;
    err:
    free(SFO);
    return nullptr;
}

uint64_t PKGInfo::getPkgSize() {
    return pkgSize;
}

std::string PKGInfo::getPath() {
    return path;
}

void PKGInfo::changeType(int type) {
    packageType = static_cast<PKGTypeENUM>(type);
}

uint32_t PKGInfo::getIconSize() {
    return iconSize;
}

std::string PKGInfo::getID() {
    return ID;
}

std::string PKGInfo::genRandom(const int len) {
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}