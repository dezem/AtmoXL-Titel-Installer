#include <fstream>
#include <iomanip>
#include "util/config.hpp"
#include "util/json.hpp"

namespace inst::config {
    std::string gAuthKey;
    std::string lastNetUrl;
    std::vector<std::string> updateInfo;
    std::string httpIndexUrl;
    std::string themeColorTextTopInfo;
    std::string themeColorTextBottomInfo;
    std::string themeColorTextMenu;
    std::string themeColorTextFile;
    std::string themeColorTextDir;
    std::string themeColorTextInstall;
    int languageSetting;
    int themeMenuFontSize;
    bool autoUpdate;
    bool deletePrompt;
    bool enableSound;
    bool enableLightning;
    bool ignoreReqVers;
    bool overClock;
    bool usbAck;
    bool validateNCAs;

    void setConfig() {
        nlohmann::json j = {
            {"autoUpdate", autoUpdate},
            {"deletePrompt", deletePrompt},
            {"enableSound", enableSound},
            {"enableLightning", enableLightning},
            {"gAuthKey", gAuthKey},
            {"ignoreReqVers", ignoreReqVers},
            {"languageSetting", languageSetting},
            {"overClock", overClock},
            {"usbAck", usbAck},
            {"validateNCAs", validateNCAs},
            {"lastNetUrl", lastNetUrl},
            {"httpIndexUrl", httpIndexUrl}
        };
        std::ofstream file(inst::config::configPath);
        file << std::setw(4) << j << std::endl;
    }

    void parseConfig() {
        try {
            std::ifstream file(inst::config::configPath);
            nlohmann::json j;
            file >> j;
            autoUpdate = j["autoUpdate"].get<bool>();
            deletePrompt = j["deletePrompt"].get<bool>();
            enableSound = j["enableSound"].get<bool>();
            enableLightning = j["enableLightning"].get<bool>();
            gAuthKey = j["gAuthKey"].get<std::string>();
            ignoreReqVers = j["ignoreReqVers"].get<bool>();
            languageSetting = j["languageSetting"].get<int>();
            overClock = j["overClock"].get<bool>();
            usbAck = j["usbAck"].get<bool>();
            validateNCAs = j["validateNCAs"].get<bool>();
            lastNetUrl = j["lastNetUrl"].get<std::string>();
            httpIndexUrl = j["httpIndexUrl"].get<std::string>();
        }
        catch (...) {
            // If loading values from the config fails, we just load the defaults and overwrite the old config
            gAuthKey = {0x41,0x49,0x7a,0x61,0x53,0x79,0x42,0x4d,0x71,0x76,0x34,0x64,0x58,0x6e,0x54,0x4a,0x4f,0x47,0x51,0x74,0x5a,0x5a,0x53,0x33,0x43,0x42,0x6a,0x76,0x66,0x37,0x34,0x38,0x51,0x76,0x78,0x53,0x7a,0x46,0x30};
            languageSetting = 99;
            autoUpdate = true;
            deletePrompt = true;
            enableSound = true;
            enableLightning = true;
            ignoreReqVers = true;
            overClock = false;
            usbAck = false;
            validateNCAs = true;
            lastNetUrl = "https://";
            httpIndexUrl = "http://";
            setConfig();
        }
    }

    void parseThemeColorConfig() {
        try {
            std::ifstream file(inst::config::themecolorPath);
            nlohmann::json j;
            file >> j;
            try {
                themeColorTextTopInfo = j["themeColorTextTopInfo"].get<std::string>();
            }
            catch (...) {
                themeColorTextTopInfo = "#FFFFFFFF";
            }
            try {
                themeColorTextBottomInfo = j["themeColorTextBottomInfo"].get<std::string>();
            }
            catch (...) {
                themeColorTextBottomInfo = "#FFFFFFFF";
            }
            try {
                themeColorTextMenu = j["themeColorTextMenu"].get<std::string>();
            }
            catch (...) {
                themeColorTextMenu = "#FFFFFFFF";
            }
            try {
                themeColorTextFile = j["themeColorTextFile"].get<std::string>();
            }
            catch (...) {
                themeColorTextFile = "#FFFFFFFF";
            }
            try {
                themeColorTextDir = j["themeColorTextDir"].get<std::string>();
            }
            catch (...) {
                themeColorTextDir = "#FFFFFFFF";
            }
            try {
                themeColorTextInstall = j["themeColorTextInstall"].get<std::string>();
            }
            catch (...) {
                themeColorTextInstall = "#FFFFFFFF";
            }
            try {
                themeMenuFontSize = j["themeMenuFontSize"].get<int>();
            }
            catch (...) {
                themeMenuFontSize = 84;
            }
        }
        catch (...) {
            // If themecolor.json is missing, load the defaults
            themeColorTextTopInfo = "#FFFFFFFF";
            themeColorTextBottomInfo = "#FFFFFFFF";
            themeColorTextMenu = "#FFFFFFFF";
            themeColorTextFile = "#FFFFFFFF";
            themeColorTextDir = "#FFFFFFFF";
            themeColorTextInstall = "#FFFFFFFF";
            themeMenuFontSize = 84;
        }
    }
}