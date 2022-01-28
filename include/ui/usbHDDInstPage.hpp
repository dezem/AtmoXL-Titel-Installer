#pragma once
#include <filesystem>
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
    class usbHDDInstPage : public pu::ui::Layout
    {
        public:
            usbHDDInstPage();
            PU_SMART_CTOR(usbHDDInstPage)
            pu::ui::elm::Menu::Ref menu;
            void startInstall();
            void onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos);
            void updateStatsThread();
            TextBlock::Ref pageInfoText;
            void drawMenuItems(bool clearItems, std::filesystem::path ourPath);
            Image::Ref titleImage;
            TextBlock::Ref appVersionText;
            TextBlock::Ref freeSpaceText;
            TextBlock::Ref batteryValueText;
        private:
            std::vector<std::filesystem::path> ourDirectories;
            std::vector<std::filesystem::path> ourFiles;
            std::vector<std::filesystem::path> selectedTitles;
            std::vector<std::pair<u64, u32>> installedTitles;
            std::vector<long unsigned int> menuIndices;
            std::filesystem::path currentDir;
            TextBlock::Ref butText;
            Rectangle::Ref topRect;
            Rectangle::Ref infoRect;
            Rectangle::Ref botRect;
            void followDirectory();
            void selectNsp(int selectedIndex);
    };
}