#include "mod/KeystrokesMod.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/input/KeyInputEvent.h"
#include "ll/api/event/render/UIRenderEvent.h"
#include "ll/api/mod/RegisterHelper.h"

#include "mc/client/gui/CaretMeasureData.h"
#include "mc/client/gui/TextAlignment.h"
#include "mc/client/gui/TextMeasureData.h"
#include "mc/client/renderer/screen/MinecraftUIRenderContext.h"
#include "mc/deps/core/math/Color.h"
#include "mc/deps/input/RectangleArea.h"

#include <array>
#include <atomic>
#include <optional>
#include <string>
#include <string_view>

namespace keystrokes {
namespace {

// Minecraft UI coordinates (not desktop physical pixels). Change these to move/resize.
constexpr float kLeft = 18.0f;
constexpr float kTop = 95.0f;
constexpr float kUnit = 36.0f;
constexpr float kGap = 4.0f;
constexpr float kSpaceHeight = 25.0f;

struct KeyState {
    int virtualKey;
    std::string_view text;
    float column;
    float row;
    float width;
    float height;
    std::atomic_bool down{false};
};

// Windows virtual-key values; ASCII uppercase letter codes and VK_SPACE (0x20).
std::array<KeyState, 5> gKeys{{
    {'W', "W", 1.0f, 0.0f, kUnit, kUnit},
    {'A', "A", 0.0f, 1.0f, kUnit, kUnit},
    {'S', "S", 1.0f, 1.0f, kUnit, kUnit},
    {'D', "D", 2.0f, 1.0f, kUnit, kUnit},
    {0x20, "SPACE", 0.0f, 2.0f, kUnit * 3.0f + 2.0f * kGap, kSpaceHeight},
}};

std::atomic_bool gVisible{true};
std::atomic_bool gF8Held{false};

RectangleArea rectAt(float x, float y, float width, float height) {
    // Important: RectangleArea layout is x0, x1, y0, y1 (not x0,y0,x1,y1).
    return RectangleArea{x, x + width, y, y + height};
}

void updateKey(ll::event::KeyInputEvent& event) {
    int const vk = event.keyCode();
    if (vk == 0x77) { // F8: toggle once per press, even if key-repeat fires.
        if (!event.isDown()) {
            gF8Held.store(false, std::memory_order_relaxed);
        } else if (!gF8Held.exchange(true, std::memory_order_relaxed)) {
            gVisible.store(!gVisible.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        return;
    }
    for (auto& key : gKeys) {
        if (key.virtualKey == vk) {
            key.down.store(event.isDown(), std::memory_order_relaxed);
            break;
        }
    }
}

void renderOverlay(ll::event::AfterUIRenderEvent& event) {
    if (!gVisible.load(std::memory_order_relaxed)) return;

    auto& ctx = event.uiRenderContext();
    mce::Color const idleBg{37, 41, 52};
    mce::Color const pressedBg{72, 119, 196};
    mce::Color const idleBorder{88, 95, 111};
    mce::Color const pressedBorder{148, 190, 255};
    mce::Color const white{255, 255, 255};

    for (auto const& key : gKeys) {
        float const x = kLeft + key.column * (kUnit + kGap);
        float const y = kTop + key.row * (kUnit + kGap);
        bool const pressed = key.down.load(std::memory_order_relaxed);
        auto const cell = rectAt(x, y, key.width, key.height);

        ctx.fillRectangle(cell, pressed ? pressedBg : idleBg, 0.86f);
        ctx.drawRectangle(cell, pressed ? pressedBorder : idleBorder, 0.85f, 1);

        // drawDebugText uses the Minecraft debug font and does not require obtaining Font*.
        // TextMeasureData fields: fontSize, linePadding, renderShadow,
        // showColorSymbol, hideHyphen, alignment.
        TextMeasureData const measure{1.1f, 0.0f, false, false, false, ui::TextAlignment::Center};
        CaretMeasureData const caret{0, false};
        float const textHeight = 15.0f;
        auto const textRect = rectAt(x + 1.0f, y + (key.height - textHeight) * 0.5f,
                                     key.width - 2.0f, textHeight);
        ctx.drawDebugText(textRect, std::string{key.text}, white, 1.0f,
                          ui::TextAlignment::Center, measure, caret);
    }

    // Flush the text batch so the labels appear at this point in the UI pass.
    ctx.flushText(0.0f, std::nullopt);
}

} // namespace

KeystrokesMod& KeystrokesMod::getInstance() {
    static KeystrokesMod instance;
    return instance;
}

bool KeystrokesMod::load() { return true; }

bool KeystrokesMod::enable() {
    auto& bus = ll::event::EventBus::getInstance();
    bus.emplaceListener<ll::event::KeyInputEvent>(updateKey);
    bus.emplaceListener<ll::event::AfterUIRenderEvent>(renderOverlay);
    getSelf().getLogger().info("Keystrokes HUD enabled. Press F8 to toggle.");
    return true;
}

bool KeystrokesMod::disable() {
    for (auto& key : gKeys) key.down.store(false, std::memory_order_relaxed);
    gF8Held.store(false, std::memory_order_relaxed);
    getSelf().getLogger().info("Keystrokes HUD disabled.");
    return true;
}

} // namespace keystrokes

LL_REGISTER_MOD(keystrokes::KeystrokesMod, keystrokes::KeystrokesMod::getInstance());
