#pragma once
#include <pu/Plutonium>
#include "ui/mainPage.hpp"
#include "ui/netInstPage.hpp"
#include "ui/sdInstPage.hpp"
#include "ui/usbInstPage.hpp"
#include "ui/usbHDDInstPage.hpp"
#include "ui/instPage.hpp"
#include "ui/optionsPage.hpp"

namespace inst::ui {
    class MainApplication : public pu::ui::Application {
        public:
            using Application::Application;
            PU_SMART_CTOR(MainApplication)
            void OnLoad() override;
            MainPage::Ref mainPage;
            netInstPage::Ref netinstPage;
            sdInstPage::Ref sdinstPage;
            usbInstPage::Ref usbinstPage;
            usbHDDInstPage::Ref usbhddinstPage;
            instPage::Ref instpage;
            optionsPage::Ref optionspage;
    };
}