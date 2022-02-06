#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/sdInstPage.hpp"
#include "sdInstall.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"
#include "nx/fs.hpp"
#include <regex>

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;
    static s32 prev_touchcount = 0;
    static std::string getFreeSpaceText = nx::fs::GetFreeStorageSpace();
    static std::string getFreeSpaceOldText = getFreeSpaceText;
    static std::string* getBatteryChargeText = inst::util::getBatteryCharge();
    static std::string* getBatteryChargeOldText = getBatteryChargeText;
    static std::vector <int> lastIndex;
    static int subPathCounter = 0;
    static bool hideInstalled = false;

    sdInstPage::sdInstPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        if (std::filesystem::exists(inst::config::appDir + "/background.png")) this->SetBackgroundImage(inst::config::appDir + "/background.png");
        else this->SetBackgroundImage("romfs:/images/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#170909FF"));
        this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#17090980"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        this->titleImage = Image::New(0, 0, "romfs:/images/logo.png");
        this->appVersionText = TextBlock::New(490, 29, "v" + inst::config::appVersion);
        this->appVersionText->SetFont("DefaultFont@42");
        this->appVersionText->SetColor(COLOR("#FFFFFFFF"));
        this->batteryValueText = TextBlock::New(700, 9, "misc.battery_charge"_lang+": " + getBatteryChargeText[0]);
        this->batteryValueText->SetFont("DefaultFont@32");
        this->batteryValueText->SetColor(COLOR(getBatteryChargeText[1]));
        this->freeSpaceText = TextBlock::New(700, 49, "misc.sd_free"_lang+": " + getFreeSpaceText);
        this->freeSpaceText->SetFont("DefaultFont@32");
        this->freeSpaceText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 109, "inst.sd.top_info"_lang);
        this->pageInfoText->SetFont("DefaultFont@30");
        this->pageInfoText->SetColor(COLOR(inst::config::themeColorTextTopInfo));
        this->butText = TextBlock::New(10, 678, "inst.sd.buttons"_lang);
        this->butText->SetFont("DefaultFont@22");
        this->butText->SetColor(COLOR(inst::config::themeColorTextBottomInfo));
        this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR("#FFFFFF00"), inst::config::themeMenuFontSize, (506 / inst::config::themeMenuFontSize));
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
        this->Add(this->menu);
        this->updateStatsThread();
        installedTitles = inst::util::listInstalledTitles();
        this->AddThread(std::bind(&sdInstPage::updateStatsThread, this));
    }

    void sdInstPage::drawMenuItems(bool clearItems, std::filesystem::path ourPath) {
        if (clearItems) this->selectedTitles = {};
        if (ourPath == "sdmc:") this->currentDir = std::filesystem::path(ourPath.string() + "/");
        else this->currentDir = ourPath;
        this->menu->ClearItems();
        this->menuIndices = {};

        try {
            this->ourDirectories = util::getDirsAtPath(this->currentDir);
            this->ourFiles = util::getDirectoryFiles(this->currentDir, {".nsp", ".nsz", ".xci", ".xcz"});
        } catch (std::exception& e) {
            this->drawMenuItems(false, this->currentDir.parent_path());
            return;
        }
        if (this->currentDir != "sdmc:/") {
            std::string itm = "..";
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(COLOR(inst::config::themeColorTextDir));
            ourEntry->SetIcon("romfs:/images/icons/folder-upload.png");
            this->menu->AddItem(ourEntry);
        }
        for (auto& file: this->ourDirectories) {
            if (file == "..") break;
            std::string itm = file.filename().string();
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(COLOR(inst::config::themeColorTextDir));
            ourEntry->SetIcon("romfs:/images/icons/folder.png");
            this->menu->AddItem(ourEntry);
        }
        for (long unsigned int i = 0; i < this->ourFiles.size(); i++) {
            auto& file = this->ourFiles[i];

            std::string itm = file.filename().string();

            if (hideInstalled and inst::util::isTitleInstalled(itm, installedTitles))
                continue;

            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(COLOR(inst::config::themeColorTextFile));
            ourEntry->SetIcon("romfs:/images/icons/checkbox-blank-outline.png");
            for (long unsigned int j = 0; j < this->selectedTitles.size(); j++) {
                if (this->selectedTitles[j] == file) {
                    ourEntry->SetIcon("romfs:/images/icons/check-box-outline.png");
                }
            }
            this->menu->AddItem(ourEntry);
            this->menuIndices.push_back(i);
        }
    }

    void sdInstPage::followDirectory() {
        int selectedIndex = this->menu->GetSelectedIndex();
        int dirListSize = this->ourDirectories.size();
        int selectNewIndex = 0;
        if (this->currentDir != "sdmc:/") {
            dirListSize++;
            selectedIndex--;
        }
        if (selectedIndex < dirListSize) {
            if (this->menu->GetItems()[this->menu->GetSelectedIndex()]->GetName() == ".." && this->menu->GetSelectedIndex() == 0) {
                this->drawMenuItems(true, this->currentDir.parent_path());
                if (subPathCounter > 0) {
                    subPathCounter--;
                    selectNewIndex = lastIndex[subPathCounter];
                    lastIndex.pop_back();
                }
            } else {
                this->drawMenuItems(true, this->ourDirectories[selectedIndex]);
                if (subPathCounter > 0) {
                    lastIndex.push_back(selectedIndex + 1);
                } else {
                    lastIndex.push_back(selectedIndex);
                }
                subPathCounter++;
            }
            this->menu->SetSelectedIndex(selectNewIndex);
        }
    }

    void sdInstPage::selectNsp(int selectedIndex) {
        int dirListSize = this->ourDirectories.size();
        if (this->currentDir != "sdmc:/") dirListSize++;

        long unsigned int nspIndex = 0;
        if (this->menuIndices.size() > 0) nspIndex = this->menuIndices[selectedIndex - dirListSize];

        if (this->menu->GetItems()[selectedIndex]->GetIcon() == "romfs:/images/icons/check-box-outline.png") {
            for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
                if (this->selectedTitles[i] == this->ourFiles[nspIndex]) this->selectedTitles.erase(this->selectedTitles.begin() + i);
            }
        } else if (this->menu->GetItems()[selectedIndex]->GetIcon() == "romfs:/images/icons/checkbox-blank-outline.png") this->selectedTitles.push_back(this->ourFiles[nspIndex]);
        else {
            this->followDirectory();
            return;
        }
        this->drawMenuItems(false, currentDir);
    }

    void sdInstPage::startInstall() {
        int dialogResult = -1;
        if (this->selectedTitles.size() == 1) {
            dialogResult = mainApp->CreateShowDialog("inst.target.desc0"_lang + inst::util::shortenString(std::filesystem::path(this->selectedTitles[0]).filename().string(), 32, true) + "inst.target.desc1"_lang, "common.cancel_desc"_lang, {"inst.target.opt0"_lang, "inst.target.opt1"_lang}, false);
        } else dialogResult = mainApp->CreateShowDialog("inst.target.desc00"_lang + std::to_string(this->selectedTitles.size()) + "inst.target.desc01"_lang, "common.cancel_desc"_lang, {"inst.target.opt0"_lang, "inst.target.opt1"_lang}, false);
        if (dialogResult == -1) return;
        nspInstStuff::installNspFromFile(this->selectedTitles, dialogResult);
        installedTitles = inst::util::listInstalledTitles();
        subPathCounter = 0;
        lastIndex.clear();
    }

    void sdInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & HidNpadButton_B) {
            if (subPathCounter > 0) {
                this->menu->SetSelectedIndex(0);
                this->followDirectory();
            } else {
                mainApp->LoadLayout(mainApp->mainPage);
            }
        }
        if ((Down & HidNpadButton_A) || (pu::ui::Application::GetTouchState().count == 0 && prev_touchcount == 1)) {
            prev_touchcount = 0;
            this->selectNsp(this->menu->GetSelectedIndex());
            if (this->ourFiles.size() == 1 && this->selectedTitles.size() == 1) {
                this->startInstall();
            }
        }
        if ((Down & HidNpadButton_Y)) {
            if (this->selectedTitles.size() == this->ourFiles.size()) this->drawMenuItems(true, currentDir);
            else {
                int topDir = 0;
                if (this->currentDir != "sdmc:/") topDir++;
                for (long unsigned int i = this->ourDirectories.size() + topDir; i < this->menu->GetItems().size(); i++) {
                    if (this->menu->GetItems()[i]->GetIcon() == "romfs:/images/icons/check-box-outline.png") continue;
                    else this->selectNsp(i);
                }
                this->drawMenuItems(false, currentDir);
            }
        }
        /* Remove help...need space
        if ((Down & HidNpadButton_X)) {
            inst::ui::mainApp->CreateShowDialog("inst.sd.help.title"_lang, "inst.sd.help.desc"_lang, {"common.ok"_lang}, true);
        }*/

        if (Down & HidNpadButton_ZL)
            this->menu->SetSelectedIndex(std::max(0, this->menu->GetSelectedIndex() - 6));
        if (Down & HidNpadButton_ZR)
            this->menu->SetSelectedIndex(std::min((s32)this->menu->GetItems().size() - 1, this->menu->GetSelectedIndex() + 6));

        if (Down & HidNpadButton_X) {
            hideInstalled = !hideInstalled;
            this->butText->SetText(hideInstalled ? "inst.sd.buttons_show"_lang : "inst.sd.buttons"_lang);
            this->drawMenuItems(true, currentDir);
            this->menu->SetSelectedIndex(0);
        }
        if (Down & HidNpadButton_Plus) {
            if (this->selectedTitles.size() == 0 && this->menu->GetItems()[this->menu->GetSelectedIndex()]->GetIcon() == "romfs:/images/icons/checkbox-blank-outline.png") {
                this->selectNsp(this->menu->GetSelectedIndex());
            }
            if (this->selectedTitles.size() > 0) this->startInstall();
        }
        if (pu::ui::Application::GetTouchState().count == 1)
            prev_touchcount = 1;
    }

    void sdInstPage::updateStatsThread() {
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
