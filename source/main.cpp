#include <thread>
#include "switch.h"
#include "util/error.hpp"
#include "ui/MainApplication.hpp"
#include "util/util.hpp"
#include "util/config.hpp"

using namespace pu::ui::render;
int main(int argc, char* argv[])
{
    inst::util::initApp();
    try {
        auto renderer = Renderer::New(pu::ui::render::RendererInitOptions(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER,
            pu::ui::render::RendererHardwareFlags).WithIMG(pu::ui::render::IMGAllFlags).WithMixer(pu::ui::render::MixerAllFlags).WithTTF().WithDefaultFontSize(30).WithRomfs());
        auto main = inst::ui::MainApplication::New(renderer);
        std::thread updateThread;
        if (inst::config::autoUpdate && inst::util::getIPAddress() != "1.0.0.127") updateThread = std::thread(inst::util::checkForAppUpdate);
        main->Prepare();
        main->ShowWithFadeIn();
        updateThread.join();
    } catch (std::exception& e) {
        LOG_DEBUG("An error occurred:\n%s", e.what());
    }
    inst::util::deinitApp();
    return 0;
}
