// Stato dei tasti del pannello, condiviso fra tastiera, mouse e clic agganciati.
// Nessun timer per la tastiera: una pressione dura fino al suo rilascio fisico.
#pragma once

#include "dgl/Base.hpp"
#include "panel_layout.hpp"

#include <cstring>
#include <map>
#include <set>
#include <utility>

namespace dnui {

inline unsigned keyForName(const char* name)
{
    using namespace DGL_NAMESPACE;
    static const struct { const char* name; unsigned key; } special[] = {
        {"Return", kKeyEnter}, {"KP_Enter", kKeyPadEnter}, {"BackSpace", kKeyBackspace},
        {"Up", kKeyUp}, {"Down", kKeyDown}, {"Left", kKeyLeft}, {"Right", kKeyRight},
        {"Tab", kKeyTab}, {"space", kKeySpace}, {"period", '.'},
        {"Alt_L", kKeyAltL}, {"Alt_R", kKeyAltR}, {"Option_L", kKeyAltL}, {"Option_R", kKeyAltR},
        {"F1", kKeyF1}, {"F2", kKeyF2}, {"F3", kKeyF3}, {"F4", kKeyF4},
        {"F5", kKeyF5}, {"F6", kKeyF6}, {"F7", kKeyF7}, {"F8", kKeyF8},
        {"F9", kKeyF9}, {"F10", kKeyF10}, {"F11", kKeyF11}, {"F12", kKeyF12}
    };
    for (const auto& s : special) if (!std::strcmp(s.name, name)) return s.key;
    return name[0] && !name[1] ? unsigned(static_cast<unsigned char>(name[0])) : 0;
}

class PanelInput {
public:
    enum Source { Keyboard, Mouse, Latch, Encoder };

    bool isDown(int code) const
    {
        for (const auto& held : fHeld) if (held.second == code) return true;
        return false;
    }

    template<class Send>
    void press(Source source, unsigned id, int code, Send send)
    {
        if (fHeld.count({source, id})) return;             // autorepeat o doppio clic
        const bool wasDown = isDown(code);
        fHeld[{source, id}] = code;
        if (!wasDown) send(0, code);
    }

    template<class Send>
    bool release(Source source, unsigned id, Send send)
    {
        const auto it = fHeld.find({source, id});
        if (it == fHeld.end()) return false;
        const int code = it->second;
        fHeld.erase(it);
        if (!isDown(code)) send(1, code);
        return true;
    }

    template<class Send>
    bool keyboard(bool down, unsigned key, unsigned keycode, unsigned modifiers, Send send)
    {
        if (key >= 'A' && key <= 'Z') key += 'a' - 'A';
        // Il keycode fisico non cambia se Option/Shift viene rilasciato per primo.
        // Alcuni host non lo forniscono: in quel caso si usa il tasto normalizzato.
        const unsigned id = keycode ? keycode : (key | 0x80000000u);
        if (!down) return release(Keyboard, id, send);     // anche se ora Ctrl/Cmd e' premuto
        if (modifiers & (DGL_NAMESPACE::kModifierControl | DGL_NAMESPACE::kModifierSuper)) return false;
        if (fHeld.count({Keyboard, id})) return true;
        for (const auto& binding : dtl::kKeyboard) {
            if (keyForName(binding.name) != key) continue;
            press(Keyboard, id, binding.code, send);
            return true;
        }
        return false;
    }

    template<class Send>
    void releaseAll(Send send)
    {
        std::set<int> codes;
        for (const auto& held : fHeld) codes.insert(held.second);
        fHeld.clear();
        for (int code : codes) send(1, code);
    }

private:
    std::map<std::pair<Source, unsigned>, int> fHeld;
};

} // namespace dnui
