#include <filesystem>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/instPage.hpp"
#include "ui/optionsPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/curl.hpp"
#include "util/unzip.hpp"
#include "util/lang.hpp"
#include "ui/instPage.hpp"
#include "nx/fs.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;
    std::vector<std::string> languageStrings = {"English", "日本語", "Français", "Deutsch", "Italiano", "Español", "한국전통", "Português", "Русский", "簡体中文","繁體中文"};
    static s32 prev_touchcount = 0;
    static std::string getFreeSpaceText = nx::fs::GetFreeStorageSpace();
    static std::string getFreeSpaceOldText = getFreeSpaceText;
    static std::string* getBatteryChargeText = inst::util::getBatteryCharge();
    static std::string* getBatteryChargeOldText = getBatteryChargeText;

    optionsPage::optionsPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        if (std::filesystem::exists(inst::config::appDir + "/background.png")) this->SetBackgroundImage(inst::config::appDir + "/background.png");
        else this->SetBackgroundImage("romfs:/images/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#170909FF"));
        this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#17090980"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        this->titleImage = Image::New(0, 0, "romfs:/images/logo.png");
        this->appVersionText = TextBlock::New(490, 29, "v" + inst::config::appVersion, 42);
        this->appVersionText->SetColor(COLOR("#FFFFFFFF"));
        this->batteryValueText = TextBlock::New(700, 9, "misc.battery_charge"_lang+": " + getBatteryChargeText[0], 32);
        this->batteryValueText->SetColor(COLOR(getBatteryChargeText[1]));
        this->freeSpaceText = TextBlock::New(700, 49, "misc.sd_free"_lang+": " + getFreeSpaceText, 32);
        this->freeSpaceText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 109, "options.title"_lang, 30);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->butText = TextBlock::New(10, 678, "options.buttons"_lang, 22);
        this->butText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR("#FFFFFF00"), 84, (506 / 84));
        this->menu->SetOnFocusColor(COLOR("#00000033"));
        this->menu->SetScrollbarColor(COLOR("#17090980"));
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->botRect);
        this->Add(this->titleImage);
        this->Add(this->appVersionText);
        this->Add(this->batteryValueText);
        this->Add(this->freeSpaceText);
        this->Add(this->butText);
        this->Add(this->pageInfoText);
        this->setMenuText();
        this->Add(this->menu);
        this->updateStatsThread();
        this->AddThread(std::bind(&optionsPage::updateStatsThread, this));
    }

    void optionsPage::askToUpdate(std::vector<std::string> updateInfo) {
            if (!mainApp->CreateShowDialog("options.update.title"_lang, "options.update.desc0"_lang + updateInfo[0] + "options.update.desc1"_lang, {"options.update.opt0"_lang, "common.cancel"_lang}, false)) {
                inst::ui::instPage::loadInstallScreen();
                inst::ui::instPage::setTopInstInfoText("options.update.top_info"_lang + updateInfo[0]);
                inst::ui::instPage::setInstBarPerc(0);
                inst::ui::instPage::setInstInfoText("options.update.bot_info"_lang + updateInfo[0]);
                try {
                    std::string downloadName = inst::config::appDir + "/temp_download.zip";
                    inst::curl::downloadFile(updateInfo[1], downloadName.c_str(), 0, true);
                    romfsExit();
                    inst::ui::instPage::setInstInfoText("options.update.bot_info2"_lang + updateInfo[0]);
                    inst::zip::extractFile(downloadName, "sdmc:/");
                    std::filesystem::remove(downloadName);
                    mainApp->CreateShowDialog("options.update.complete"_lang, "options.update.end_desc"_lang, {"common.ok"_lang}, false);
                } catch (...) {
                    mainApp->CreateShowDialog("options.update.failed"_lang, "options.update.end_desc"_lang, {"common.ok"_lang}, false);
                }
                mainApp->FadeOut();
                mainApp->Close();
            }
        return;
    }

    std::string optionsPage::getMenuOptionIcon(bool ourBool) {
        if(ourBool) return "romfs:/images/icons/check-box-outline.png";
        else return "romfs:/images/icons/checkbox-blank-outline.png";
    }

    std::string optionsPage::getMenuLanguage(int ourLangCode) {
        switch (ourLangCode) {
            case 1:
            case 12:
                return languageStrings[0];
            case 0:
                return languageStrings[1];
            case 2:
            case 13:
                return languageStrings[2];
            case 3:
                return languageStrings[3];
            case 4:
                return languageStrings[4];
            case 5:
            case 14:
                return languageStrings[5];
            case 7:
                return languageStrings[6];
            case 9:
                return languageStrings[7];
            case 10:
                return languageStrings[8];
            case 6:
                return languageStrings[9];
            case 11:
                return languageStrings[10];
            default:
                return "options.language.system_language"_lang;
        }
    }

    void optionsPage::setMenuText() {
        this->menu->ClearItems();
        auto ignoreFirmOption = pu::ui::elm::MenuItem::New("options.menu_items.ignore_firm"_lang);
        ignoreFirmOption->SetColor(COLOR("#FFFFFFFF"));
        ignoreFirmOption->SetIcon(this->getMenuOptionIcon(inst::config::ignoreReqVers));
        this->menu->AddItem(ignoreFirmOption);
        auto validateOption = pu::ui::elm::MenuItem::New("options.menu_items.nca_verify"_lang);
        validateOption->SetColor(COLOR("#FFFFFFFF"));
        validateOption->SetIcon(this->getMenuOptionIcon(inst::config::validateNCAs));
        this->menu->AddItem(validateOption);
        auto overclockOption = pu::ui::elm::MenuItem::New("options.menu_items.boost_mode"_lang);
        overclockOption->SetColor(COLOR("#FFFFFFFF"));
        overclockOption->SetIcon(this->getMenuOptionIcon(inst::config::overClock));
        this->menu->AddItem(overclockOption);
        auto deletePromptOption = pu::ui::elm::MenuItem::New("options.menu_items.ask_delete"_lang);
        deletePromptOption->SetColor(COLOR("#FFFFFFFF"));
        deletePromptOption->SetIcon(this->getMenuOptionIcon(inst::config::deletePrompt));
        this->menu->AddItem(deletePromptOption);
        auto enableSoundOption = pu::ui::elm::MenuItem::New("options.menu_items.enableSound"_lang);
        enableSoundOption->SetColor(COLOR("#FFFFFFFF"));
        enableSoundOption->SetIcon(this->getMenuOptionIcon(inst::config::enableSound));
        this->menu->AddItem(enableSoundOption);
        auto enableLightningOption = pu::ui::elm::MenuItem::New("options.menu_items.enableLightning"_lang);
        enableLightningOption->SetColor(COLOR("#FFFFFFFF"));
        enableLightningOption->SetIcon(this->getMenuOptionIcon(inst::config::enableLightning));
        this->menu->AddItem(enableLightningOption);
        auto autoUpdateOption = pu::ui::elm::MenuItem::New("options.menu_items.auto_update"_lang);
        autoUpdateOption->SetColor(COLOR("#FFFFFFFF"));
        autoUpdateOption->SetIcon(this->getMenuOptionIcon(inst::config::autoUpdate));
        this->menu->AddItem(autoUpdateOption);
        auto languageOption = pu::ui::elm::MenuItem::New("options.menu_items.language"_lang + this->getMenuLanguage(inst::config::languageSetting));
        languageOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(languageOption);
        auto updateOption = pu::ui::elm::MenuItem::New("options.menu_items.check_update"_lang);
        updateOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(updateOption);
        auto creditsOption = pu::ui::elm::MenuItem::New("options.menu_items.credits"_lang);
        creditsOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(creditsOption);
    }

    void optionsPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & HidNpadButton_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if ((Down & HidNpadButton_A) || (pu::ui::Application::GetTouchState().count == 0 && prev_touchcount == 1)) {
            prev_touchcount = 0;
            std::string keyboardResult;
            int rc;
            std::vector<std::string> downloadUrl;
            std::vector<std::string> languageList;
            switch (this->menu->GetSelectedIndex()) {
                case 0:
                    inst::config::ignoreReqVers = !inst::config::ignoreReqVers;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 1:
                    if (inst::config::validateNCAs) {
                        if (inst::ui::mainApp->CreateShowDialog("options.nca_warn.title"_lang, "options.nca_warn.desc"_lang, {"common.cancel"_lang, "options.nca_warn.opt1"_lang}, false) == 1) inst::config::validateNCAs = false;
                    } else inst::config::validateNCAs = true;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 2:
                    inst::config::overClock = !inst::config::overClock;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 3:
                    inst::config::deletePrompt = !inst::config::deletePrompt;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 4:
                    inst::config::enableSound = !inst::config::enableSound;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 5:
                    inst::config::enableLightning = !inst::config::enableLightning;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 6:
                    inst::config::autoUpdate = !inst::config::autoUpdate;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 7:
                    languageList = languageStrings;
                    languageList.push_back("options.language.system_language"_lang);
                    rc = inst::ui::mainApp->CreateShowDialog("options.language.title"_lang, "options.language.desc"_lang, languageList, false);
                    if (rc == -1) break;
                    switch(rc) {
                        case 0:
                            inst::config::languageSetting = 1;
                            break;
                        case 1:
                            inst::config::languageSetting = 0;
                            break;
                        case 2:
                            inst::config::languageSetting = 2;
                            break;
                        case 3:
                            inst::config::languageSetting = 3;
                            break;
                        case 4:
                            inst::config::languageSetting = 4;
                            break;
                        case 5:
                            inst::config::languageSetting = 14;
                            break;
                        case 6:
                            inst::config::languageSetting = 7;
                            break;
                        case 7:
                            inst::config::languageSetting = 9;
                            break;
                        case 8:
                            inst::config::languageSetting = 10;
                            break;
                        case 9:
                            inst::config::languageSetting = 6;
                            break;
                        case 10:
                            inst::config::languageSetting = 11;
                            break;
                        default:
                            inst::config::languageSetting = 99;
                    }
                    inst::config::setConfig();
                    mainApp->FadeOut();
                    mainApp->Close();
                    break;
                case 8:
                    if (inst::util::getIPAddress() == "1.0.0.127") {
                        inst::ui::mainApp->CreateShowDialog("main.net.title"_lang, "main.net.desc"_lang, {"common.ok"_lang}, true);
                        break;
                    }
                    downloadUrl = inst::util::checkForAppUpdate();
                    if (!downloadUrl.size()) {
                        mainApp->CreateShowDialog("options.update.title_check_fail"_lang, "options.update.desc_check_fail"_lang, {"common.ok"_lang}, false);
                        break;
                    }
                    this->askToUpdate(downloadUrl);
                    break;
                case 9:
                    inst::ui::mainApp->CreateShowDialog("options.credits.title"_lang, "options.credits.desc"_lang, {"common.close"_lang}, true);
                    break;
                default:
                    break;
            }
        }
        if (pu::ui::Application::GetTouchState().count == 1)
            prev_touchcount = 1;
    }

    void optionsPage::updateStatsThread() {
        getFreeSpaceText = nx::fs::GetFreeStorageSpace();
        if (getFreeSpaceOldText != getFreeSpaceText) {
            getFreeSpaceOldText = getFreeSpaceText;
            mainApp->instpage->freeSpaceText->SetText("misc.sd_free"_lang+": " + getFreeSpaceText);
            mainApp->usbhddinstPage->freeSpaceText->SetText("misc.sd_free"_lang+": " + getFreeSpaceText);
            mainApp->sdinstPage->freeSpaceText->SetText("misc.sd_free"_lang+": " + getFreeSpaceText);
            mainApp->netinstPage->freeSpaceText->SetText("misc.sd_free"_lang+": " + getFreeSpaceText);
            mainApp->usbinstPage->freeSpaceText->SetText("misc.sd_free"_lang+": " + getFreeSpaceText);
            mainApp->mainPage->freeSpaceText->SetText("misc.sd_free"_lang+": " + getFreeSpaceText);
            mainApp->optionspage->freeSpaceText->SetText("misc.sd_free"_lang+": " + getFreeSpaceText);
        }

        getBatteryChargeText = inst::util::getBatteryCharge();
        if (getBatteryChargeOldText[0] != getBatteryChargeText[0]) {
            getBatteryChargeOldText = getBatteryChargeText;

            mainApp->instpage->batteryValueText->SetColor(COLOR(getBatteryChargeText[1]));
            mainApp->usbhddinstPage->batteryValueText->SetColor(COLOR(getBatteryChargeText[1]));
            mainApp->sdinstPage->batteryValueText->SetColor(COLOR(getBatteryChargeText[1]));
            mainApp->netinstPage->batteryValueText->SetColor(COLOR(getBatteryChargeText[1]));
            mainApp->usbinstPage->batteryValueText->SetColor(COLOR(getBatteryChargeText[1]));
            mainApp->mainPage->batteryValueText->SetColor(COLOR(getBatteryChargeText[1]));
            mainApp->optionspage->batteryValueText->SetColor(COLOR(getBatteryChargeText[1]));

            mainApp->instpage->batteryValueText->SetText("misc.battery_charge"_lang+": " + getBatteryChargeText[0]);
            mainApp->usbhddinstPage->batteryValueText->SetText("misc.battery_charge"_lang+": " + getBatteryChargeText[0]);
            mainApp->sdinstPage->batteryValueText->SetText("misc.battery_charge"_lang+": " + getBatteryChargeText[0]);
            mainApp->netinstPage->batteryValueText->SetText("misc.battery_charge"_lang+": " + getBatteryChargeText[0]);
            mainApp->usbinstPage->batteryValueText->SetText("misc.battery_charge"_lang+": " + getBatteryChargeText[0]);
            mainApp->mainPage->batteryValueText->SetText("misc.battery_charge"_lang+": " + getBatteryChargeText[0]);
            mainApp->optionspage->batteryValueText->SetText("misc.battery_charge"_lang+": " + getBatteryChargeText[0]);
        }
    }
}
