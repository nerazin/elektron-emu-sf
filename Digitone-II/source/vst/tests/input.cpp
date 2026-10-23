// Firmware-free regression tests, using the same input code and DPF keys as the UI.
#include "DigitoneInput.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace DGL_NAMESPACE;
using Input = dnui::PanelInput;
using Event = std::pair<int, int>;

#define CHECK(condition) do { if (!(condition)) { \
    std::cerr << __FILE__ << ':' << __LINE__ << ": " #condition "\n"; \
    std::exit(1); } } while (false)

struct Fixture {
    Input input;
    std::vector<Event> events;
    auto send() { return [this](int kind, int code) { events.emplace_back(kind, code); }; }
    bool key(bool down, unsigned key, unsigned raw, unsigned mod = 0)
    {
        return input.keyboard(down, key, raw, mod, send());
    }
    void expect(std::initializer_list<Event> expected)
    {
        CHECK(events == std::vector<Event>(expected));
        events.clear();
    }
};

static void holdsAndChords()
{
    Fixture f;
    CHECK(f.key(true, '1', 10));
    for (int i = 0; i < 10000; ++i) CHECK(f.key(true, '1', 10));
    f.expect({{0, 25}});                                   // one press, never a timed tap
    CHECK(f.input.isDown(25));
    CHECK(f.key(false, '1', 10));
    CHECK(!f.key(false, '1', 10));                          // stray keyup
    CHECK(!f.input.isDown(25));
    f.expect({{1, 25}});

    CHECK(f.key(true, kKeyAltL, 11, kModifierAlt));
    CHECK(f.key(true, kKeyTab, 12, kModifierAlt));
    CHECK(f.key(true, 'r', 13, kModifierAlt));               // TRIG 12, not RECORD
    f.expect({{0, 17}, {0, 16}, {0, 36}});
    CHECK(f.key(false, kKeyAltL, 11));                      // release modifier first
    CHECK(f.input.isDown(16) && f.input.isDown(36));
    CHECK(f.key(false, kKeyTab, 12));
    CHECK(f.key(false, 'r', 13));
    f.expect({{1, 17}, {1, 16}, {1, 36}});
}

static void physicalKeysAndHostShortcuts()
{
    Fixture f;
    CHECK(f.key(true, 'Q', 20));
    CHECK(f.key(false, 'q', 20, kModifierControl));          // Ctrl pressed after note-on
    f.expect({{0, 33}, {1, 33}});
    CHECK(f.key(true, 'w', 21, kModifierAlt));
    CHECK(f.key(false, 0x2211, 21));                        // Option/layout changed the symbol
    f.expect({{0, 34}, {1, 34}});
    CHECK(f.key(true, 'e', 22));
    CHECK(f.key(true, 'r', 22));                            // repeat with a changed symbol
    CHECK(f.key(false, 'r', 22));
    f.expect({{0, 35}, {1, 35}});
    CHECK(f.key(true, 'J', 0));                             // host without raw keycodes
    CHECK(f.key(false, 'j', 0));
    f.expect({{0, 19}, {1, 19}});
    for (unsigned mod : {unsigned(kModifierControl), unsigned(kModifierSuper)}) {
        CHECK(!f.key(true, '1', 23, mod));
        CHECK(!f.key(true, 's', 24, mod));
        CHECK(!f.key(false, '1', 23));
    }
    CHECK(!f.key(true, '?', 25));
    CHECK(!f.key(false, '?', 25));
    f.expect({});
}

static void aliasesAndInputSources()
{
    Fixture f;
    CHECK(f.key(true, kKeyAltL, 30));
    CHECK(f.key(true, kKeyAltR, 31));
    CHECK(f.key(false, kKeyAltL, 30));
    CHECK(f.input.isDown(17));
    f.expect({{0, 17}});                                   // right Alt still holds FUNC
    CHECK(f.key(false, kKeyAltR, 31));
    f.expect({{1, 17}});

    CHECK(f.key(true, '=', 32));
    CHECK(f.key(true, '+', 33));
    CHECK(f.key(false, '=', 32));
    CHECK(f.input.isDown(52));
    CHECK(f.key(false, '+', 33));
    f.expect({{0, 52}, {1, 52}});

    CHECK(f.key(true, '1', 34));
    f.input.press(Input::Mouse, 0, 25, f.send());
    CHECK(f.key(false, '1', 34));
    CHECK(f.input.isDown(25));
    f.expect({{0, 25}});                                   // mouse still holds TRIG 1
    CHECK(f.input.release(Input::Mouse, 0, f.send()));
    f.expect({{1, 25}});

    f.input.press(Input::Latch, 17, 17, f.send());
    CHECK(f.key(true, kKeyAltL, 35));
    CHECK(f.input.release(Input::Latch, 17, f.send()));      // mouse latch timer expires
    CHECK(f.input.isDown(17));
    CHECK(f.key(false, kKeyAltL, 35));
    f.expect({{0, 17}, {1, 17}});

    f.input.press(Input::Encoder, 41, 41, f.send());
    CHECK(f.key(true, 'z', 36));
    CHECK(f.input.release(Input::Encoder, 41, f.send()));    // mouse encoder tap expires
    CHECK(f.input.isDown(41));
    CHECK(f.key(false, 'z', 36));
    f.expect({{0, 41}, {1, 41}});
}

static void focusAndClose()
{
    Fixture f;
    CHECK(f.key(true, kKeyAltL, 40));
    CHECK(f.key(true, '1', 41));
    CHECK(f.key(true, 'x', 42));
    f.input.press(Input::Mouse, 0, 25, f.send());
    f.input.press(Input::Latch, 16, 16, f.send());
    f.input.press(Input::Encoder, 43, 43, f.send());
    f.events.clear();
    f.input.releaseAll(f.send());                           // focus loss, Escape or UI teardown
    f.expect({{1, 16}, {1, 17}, {1, 25}, {1, 42}, {1, 43}});
    for (int code = 1; code <= 54; ++code) CHECK(!f.input.isDown(code));
    CHECK(!f.key(false, '1', 41));
    f.input.releaseAll(f.send());
    f.expect({});
    CHECK(f.key(true, '1', 41));                            // focus returned: no stale repeat state
    CHECK(f.key(false, '1', 41));
    f.expect({{0, 25}, {1, 25}});
}

static void completeLayout()
{
    Fixture f;
    std::set<int> mapped;
    std::map<unsigned, int> keys;
    unsigned raw = 100;
    for (const auto& binding : dtl::kKeyboard) {
        const unsigned key = dnui::keyForName(binding.name);
        CHECK(key != 0);
        CHECK(!keys.count(key) || keys[key] == binding.code);
        keys[key] = binding.code;
        mapped.insert(binding.code);
        CHECK(f.key(true, key, ++raw));
        CHECK(f.input.isDown(binding.code));
        CHECK(f.key(false, key, raw));
        f.expect({{0, binding.code}, {1, binding.code}});
    }
    for (const auto& button : dtl::kKeys) CHECK(mapped.count(button.code));
    for (const auto& encoder : dtl::kEncoders) CHECK(mapped.count(encoder.push));
    CHECK(keys[kKeyBackspace] == 12);
    CHECK(!keys.count(kKeyEscape));                         // Escape is cleanup, NO is Backspace
    CHECK(keys['r'] == 36 && keys['j'] == 19);
    CHECK(keys[kKeyPadEnter] == 10);
}

int main()
{
    holdsAndChords();
    physicalKeysAndHostShortcuts();
    aliasesAndInputSources();
    focusAndClose();
    completeLayout();
    std::cout << "Digitone II input tests passed\n";
}
