#include <imgui_impl_sdlrenderer3.h>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Common.h"
#include "debug/DebugOverlay.h"
#include "hal/u8g2_hal_sdl.h"
#include "model/AppContext.h"
#include "ui/Device.h"
#include "ui/UIWidget.h"

#include "u8g2.h"

#include "PushButton.h"

#include "ui/widgets/Button.h"
#include "ui/widgets/D_Pad.h"

constexpr unsigned int WINDOW_WIDTH =
    (U8G2_SCREEN_WIDTH * U8G2_SCREEN_FACTOR + 3 * U8G2_SCREEN_PADDING + 2 * BUTTON_WIDTH +
     DPAD_WIDTH + 2 * U8G2_SCREEN_PADDING);
constexpr unsigned int WINDOW_HEIGHT = (U8G2_SCREEN_HEIGHT * U8G2_SCREEN_FACTOR + 50);

std::shared_ptr<Device> device;
std::vector<std::shared_ptr<UIWidget>> widgets;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if(SDL_Init(SDL_INIT_VIDEO) == false) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL! -> %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if(TTF_Init() == false) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize TTF! -> %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const auto win = createWindow("System Control (Simulator)", WINDOW_WIDTH, WINDOW_HEIGHT);
    if(!win) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window! -> %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderVSync(win->renderer(), SDL_RENDERER_VSYNC_ADAPTIVE);
    SDL_SetWindowPosition(win->window(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(win->window());

    const auto context = new AppContext(win);
    *appstate = context;

    device = std::make_shared<Device>(context);
    widgets.push_back(device);

    DebugOverlay::init(context);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    const auto context = static_cast<AppContext*>(appstate);
    DebugOverlay::update(context, event);

    switch(event->type) {
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        /// multi window
        if(SDL_GetWindowID(context->window()) == event->window.windowID) {
            return SDL_APP_SUCCESS;
        }
        break;

    case SDL_EVENT_QUIT:
        /// single window application
        return SDL_APP_SUCCESS;

    case SDL_EVENT_KEY_DOWN:
        switch(event->key.key) {
        case SDLK_ESCAPE:
            return SDL_APP_SUCCESS;

        case SDLK_UP:
            device->onButtonClicked(BUTTON_UP);
            break;

        case SDLK_DOWN:
            device->onButtonClicked(BUTTON_DOWN);
            break;

        case SDLK_LEFT:
            device->onButtonClicked(BUTTON_LEFT);
            break;

        case SDLK_RIGHT:
            device->onButtonClicked(BUTTON_RIGHT);
            break;

        case SDLK_RETURN:
            device->onButtonClicked(BUTTON_SELECT);
            break;

        case SDLK_BACKSPACE:
            device->onButtonClicked(BUTTON_BACK);
            break;

        default:
            break;
        }
        break;

    case SDL_EVENT_KEY_UP:
        if(event->key.key == SDLK_LSHIFT) {
            DebugOverlay::show_debug_window = !DebugOverlay::show_debug_window;
        }
        break;

    case SDL_EVENT_MOUSE_MOTION:
        //device->hitTest(&event->motion);
        break;

    default: {
        if(DebugOverlay::show_unhandled_events) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Unused event: %d", event->type);
        }
    } break;
    }

    // return continue to continue
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    const auto context = static_cast<AppContext*>(appstate);

    /// render main window
    SDL_SetRenderDrawColor(context->renderer(), 0, 0, 0, 255);
    SDL_RenderClear(context->renderer());

    for(const auto& widget : widgets) {
        widget->render();
    }
    DebugOverlay::render(context);

    SDL_RenderPresent(context->renderer());

    /// render led matrix
    context->matrix_render();

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    DebugOverlay::cleanup();

    free(appstate);

    // SDL will clean up the window/renderer for us.
    TTF_Quit();
}
