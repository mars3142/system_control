#include "D_Pad.h"
#include "ui/widgets/D_Pad.h"

#include "../../manager/ResourceManager.h"

D_Pad::D_Pad(void *appState, const float x, const float y, const float width, std::function<void(Direction)> callback)
    : UIWidget(appState), m_x(x), m_y(y), m_width(width), m_callback(std::move(callback))
{
}

void D_Pad::Render(const uint64_t dt) const
{
    const auto dPad =
        ResourceManager::Instance().GetTextureByName(GetContext()->MainRenderer(), "d-pad_normal.png");

    auto dst = SDL_FRect(m_x, m_y, m_width, m_width);
    SDL_RenderTexture(GetContext()->MainRenderer(), dPad, nullptr, &dst);
    SDL_DestroyTexture(dPad);

    if (m_isPressed != Direction::NONE)
    {
        const auto overlay =
            ResourceManager::Instance().GetTextureByName(GetContext()->MainRenderer(), "button_pressed_overlay.png");
        switch (m_isPressed)
        {
        case Direction::UP:
            dst = SDL_FRect(m_x + m_width / 3.0f, m_y, m_width / 3.0f, m_width / 3.0f);
            break;

        case Direction::DOWN:
            dst = SDL_FRect(m_x + m_width / 3.0f, m_y + 2 * m_width / 3.0f, m_width / 3.0f, m_width / 3.0f);
            break;

        case Direction::LEFT:
            dst = SDL_FRect(m_x, m_y + m_width / 3.0f, m_width / 3.0f, m_width / 3.0f);
            break;

        case Direction::RIGHT:
            dst = SDL_FRect(m_x + 2 * m_width / 3.0f, m_y + m_width / 3.0f, m_width / 3.0f, m_width / 3.0f);
            break;

        case Direction::NONE:
            break;
        }
        SDL_RenderTexture(GetContext()->MainRenderer(), overlay, nullptr, &dst);
        SDL_DestroyTexture(overlay);
    }
}

bool D_Pad::IsHit(const int mouse_x, const int mouse_y) const
{
    const auto fx = static_cast<float>(mouse_x);
    const auto fy = static_cast<float>(mouse_y);
    return (fx >= m_x && fx <= (m_x + m_width) &&
            fy >= m_y && fy <= (m_y + m_width));
}

D_Pad::Direction D_Pad::GetDirectionFromTap(const float local_x, const float local_y) const
{
    const float segment = m_width / 3.0f;

    int col = -1;
    if (local_x < segment)
        col = 0;
    else if (local_x < 2 * segment)
        col = 1;
    else if (local_x <= m_width)
        col = 2;
    else
        return Direction::NONE;

    int row = -1;
    if (local_y < segment)
        row = 0;
    else if (local_y < 2 * segment)
        row = 1;
    else if (local_y <= m_width)
        row = 2;
    else
        return Direction::NONE;

    if (col == 1 && row == 0)
        return Direction::UP;
    if (col == 1 && row == 2)
        return Direction::DOWN;
    if (col == 0 && row == 1)
        return Direction::LEFT;
    if (col == 2 && row == 1)
        return Direction::RIGHT;

    return Direction::NONE;
}

void D_Pad::OnTap(const int mouse_x, const int mouse_y)
{
    if (m_callback)
    {
        const auto local_x = static_cast<float>(mouse_x) - m_x;

        if (const auto local_y = static_cast<float>(mouse_y) - m_y;
            local_x >= 0 && local_x <= m_width && local_y >= 0 && local_y <= m_width)
        {
            m_isPressed = GetDirectionFromTap(local_x, local_y);
            m_callback(m_isPressed);
        }
    }
}

void D_Pad::ReleaseTap(const int mouse_x, const int mouse_y)
{
    m_isPressed = Direction::NONE;
}