#include "app_task.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "hal/u8g2_esp32_hal.h"
#include "u8g2.h"

#include "button_handling.h"
#include "common/InactivityTracker.h"
#include "hal_esp32/PersistenceManager.h"
#include "ui/ScreenSaver.h"
#include "ui/SplashScreen.h"

#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define PIN_SDA GPIO_NUM_35
#define PIN_SCL GPIO_NUM_36
#else
/// just dummy pins, because of compile check
#define PIN_SDA GPIO_NUM_20
#define PIN_SCL GPIO_NUM_21
#endif
#define PIN_RST GPIO_NUM_NC

static const char *TAG = "app_task";

u8g2_t u8g2;
uint8_t last_value = 0;
menu_options_t options;
uint8_t received_signal;

std::shared_ptr<Widget> m_widget;
std::vector<std::shared_ptr<Widget>> m_history;
std::unique_ptr<InactivityTracker> m_inactivityTracker;

extern QueueHandle_t buttonQueue;

static void setup_screen(void)
{
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.bus.i2c.sda = PIN_SDA;
    u8g2_esp32_hal.bus.i2c.scl = PIN_SCL;
    u8g2_esp32_hal.reset = PIN_RST;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_SetI2CAddress(&u8g2.u8x8, 0x3C * 2);

    ESP_LOGI(TAG, "u8g2_InitDisplay");
    u8g2_InitDisplay(&u8g2);

    ESP_LOGI(TAG, "u8g2_SetPowerSave");
    u8g2_SetPowerSave(&u8g2, 0);
}

void setScreen(const std::shared_ptr<Widget> &screen)
{
    if (screen != nullptr)
    {
        m_widget = screen;
        m_history.clear();
        m_history.emplace_back(m_widget);
        m_widget->enter();
    }
}

void pushScreen(const std::shared_ptr<Widget> &screen)
{
    if (screen != nullptr)
    {
        if (m_widget)
        {
            m_widget->pause();
        }
        m_widget = screen;
        m_widget->enter();
        m_history.emplace_back(m_widget);
    }
}

void popScreen()
{
    if (m_history.size() >= 2)
    {
        m_history.pop_back();
        if (m_widget)
        {
            m_widget->exit();
        }
        m_widget = m_history.back();
        m_widget->resume();
    }
}

static void init_ui(void)
{
    options = {
        .u8g2 = &u8g2,
        .setScreen = [](const std::shared_ptr<Widget> &screen) { setScreen(screen); },
        .pushScreen = [](const std::shared_ptr<Widget> &screen) { pushScreen(screen); },
        .popScreen = []() { popScreen(); },
        .onButtonClicked = nullptr,
        .persistenceManager = std::make_shared<PersistenceManager>(),
    };
    m_widget = std::make_shared<SplashScreen>(&options);
    m_inactivityTracker = std::make_unique<InactivityTracker>(60000, []() {
        auto screensaver = std::make_shared<ScreenSaver>(&options);
        options.pushScreen(screensaver);
    });
}

static void handle_button(uint8_t button)
{
    m_inactivityTracker->reset();

    if (m_widget)
    {
        switch (button)
        {
        case 1:
            m_widget->onButtonClicked(ButtonType::UP);
            break;

        case 3:
            m_widget->onButtonClicked(ButtonType::LEFT);
            break;

        case 5:
            m_widget->onButtonClicked(ButtonType::RIGHT);
            break;

        case 6:
            m_widget->onButtonClicked(ButtonType::DOWN);
            break;

        case 16:
            m_widget->onButtonClicked(ButtonType::BACK);
            break;

        case 18:
            m_widget->onButtonClicked(ButtonType::SELECT);
            break;

        default:
            ESP_LOGI(TAG, "Unhandled button: %u", button);
            break;
        }
    }
}

void app_task(void *args)
{
    setup_screen();
    setup_buttons();
    init_ui();

    auto oldTime = esp_timer_get_time();

    while (true)
    {
        u8g2_ClearBuffer(&u8g2);

        if (m_widget != nullptr)
        {
            auto currentTime = esp_timer_get_time();
            auto delta = currentTime - oldTime;
            oldTime = currentTime;

            uint64_t deltaMs = delta / 1000;

            m_widget->update(deltaMs);
            m_widget->render();

            m_inactivityTracker->update(deltaMs);
        }

        u8g2_SendBuffer(&u8g2);

        if (xQueueReceive(buttonQueue, &received_signal, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            handle_button(received_signal);
        }
    }

    cleanup_buttons();
}