#include "ui/widgets/Button.h"

#include "../../manager/ResourceManager.h"
#include <functional>
#include <utility>

Button::Button(void *appState, const float x, const float y, const float width, std::function<void()> callback)
    : UIWidget(appState), m_x(x), m_y(y), m_width(width), m_callback(std::move(callback))
{
}

void Button::Render(const uint64_t dt) const
{
    const auto button =
        ResourceManager::Instance().GetTextureByName(GetContext()->MainRenderer(), "button_normal.png");
    const auto dst = SDL_FRect(m_x, m_y, m_width, m_width);
    SDL_RenderTexture(GetContext()->MainRenderer(), button, nullptr, &dst);
    SDL_DestroyTexture(button);

    if (m_isPressed)
    {
        const auto overlay =
            ResourceManager::Instance().GetTextureByName(GetContext()->MainRenderer(), "button_pressed_overlay.png");
        SDL_RenderTexture(GetContext()->MainRenderer(), overlay, nullptr, &dst);
        SDL_DestroyTexture(overlay);
    }
}

bool Button::IsHit(const int mouse_x, const int mouse_y) const
{
    const auto fx = static_cast<float>(mouse_x);
    const auto fy = static_cast<float>(mouse_y);
    return (fx >= m_x && fx <= (m_x + m_width) &&
            fy >= m_y && fy <= (m_y + m_width)) || m_isPressed;
}

void Button::OnTap(int mouse_x, int mouse_y)
{
    if (m_callback)
    {
        m_isPressed = true;
        m_callback();
    }
}

void Button::ReleaseTap(int mouse_x, int mouse_y)
{
    m_isPressed = false;
}