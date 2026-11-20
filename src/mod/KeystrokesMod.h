#pragma once

#include "ll/api/mod/NativeMod.h"

namespace keystrokes {

class KeystrokesMod {
public:
    static KeystrokesMod& getInstance();
    KeystrokesMod() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace keystrokes
