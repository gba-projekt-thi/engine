#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_string.h"
#include "bn_fixed.h"
#include "bn_vector.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_bg_palettes.h"
#include "bn_color.h"

#include "save_manager.h"
#include "save_integrity.h"

#include "common_variable_8x16_sprite_font.h"

// --- Test Game State ---
struct TestState
{
    bn::fixed player_x;
    bn::fixed player_y;
    uint8_t   level;
    uint16_t  score;
    uint32_t  flags;
};

// --- Globals ---
constexpr int MAX_TEXT_SPRITES = 256;

engine::save::SaveManager<TestState, 3> save_mgr;

int tests_passed = 0;
int tests_failed = 0;

// --- Helpers ---
void print_line(bn::sprite_text_generator& gen,
                bn::vector<bn::sprite_ptr, MAX_TEXT_SPRITES>& sprites,
                int y, const char* msg)
{
    gen.generate(4 - 120, y, msg, sprites);
}

void wait_for_a()
{
    while(! bn::keypad::a_pressed())
    {
        bn::core::update();
    }
    bn::core::update();
}

void report(bn::sprite_text_generator& gen,
            bn::vector<bn::sprite_ptr, MAX_TEXT_SPRITES>& sprites,
            int y, const char* name, bool passed)
{
    if(passed)
    {
        tests_passed++;
        bn::string<48> msg;
        msg.append("[OK] ");
        msg.append(name);
        print_line(gen, sprites, y, msg.c_str());
    }
    else
    {
        tests_failed++;
        bn::string<48> msg;
        msg.append("[X] ");
        msg.append(name);
        print_line(gen, sprites, y, msg.c_str());
    }
}

// ========================================
// PERSISTENCE TEST — runs at boot
// ========================================
void test_persistence_check(bn::sprite_text_generator& gen,
                            bn::vector<bn::sprite_ptr, MAX_TEXT_SPRITES>& sprites)
{
    sprites.clear();
    print_line(gen, sprites, -72, "== Persist ==");

    // Try to load slot 0
    TestState loaded;
    auto result = save_mgr.load(0, loaded);

    if(result == engine::save::SaveResult::OK)
    {
        // Found existing save — verify known values
        bool match = (loaded.score == 12345) &&
                     (loaded.level == 42) &&
                     (loaded.player_x == 99) &&
                     (loaded.player_y == -55);

        if(match)
        {
            print_line(gen, sprites, -52, "[OK] SAVE PERSISTED!");
            print_line(gen, sprites, -36, "Data survived reboot");
            print_line(gen, sprites, -16, "score=12345 lvl=42");
            print_line(gen, sprites, -4, "x=99 y=-55");
            print_line(gen, sprites, 20, "SELECT=erase+retest");
            print_line(gen, sprites, 36, "START=unit tests");
        }
        else
        {
            print_line(gen, sprites, -52, "[X] Wrong data!");

            bn::string<32> s;
            s.append("score=");
            s.append(bn::to_string<8>(loaded.score));
            print_line(gen, sprites, -36, s.c_str());

            print_line(gen, sprites, -16, "START=unit tests");
        }
    }
    else
    {
        // No save found — write known values
        TestState to_save;
        to_save.player_x = 99;
        to_save.player_y = -55;
        to_save.level = 42;
        to_save.score = 12345;
        to_save.flags = 0xCAFE;

        auto save_result = save_mgr.save(0, to_save);

        if(save_result == engine::save::SaveResult::OK)
        {
            print_line(gen, sprites, -52, "No save found.");
            print_line(gen, sprites, -36, "Wrote test data:");
            print_line(gen, sprites, -20, "score=12345 lvl=42");
            print_line(gen, sprites, -4, "x=99 y=-55");
            print_line(gen, sprites, 20, "NOW CLOSE + REOPEN");
            print_line(gen, sprites, 36, "to verify persist!");
        }
        else
        {
            print_line(gen, sprites, -52, "[X] Save FAILED");
        }
    }

    // Wait for START (unit tests) or SELECT (erase)
    while(true)
    {
        if(bn::keypad::start_pressed())
        {
            bn::core::update();
            return; // continue to unit tests
        }

        if(bn::keypad::select_pressed())
        {
            // Erase and rewrite
            save_mgr.erase(0);
            save_mgr.erase(1);
            save_mgr.erase(2);

            sprites.clear();
            print_line(gen, sprites, -20, "Erased all slots.");
            print_line(gen, sprites, 0, "Close + reopen ROM");
            print_line(gen, sprites, 16, "to test fresh start");

            while(true)
            {
                bn::core::update();
            }
        }

        bn::core::update();
    }
}

// ========================================
// TEST SUITE 1: CRC32 & Integrity
// ========================================
void test_integrity(bn::sprite_text_generator& gen,
                    bn::vector<bn::sprite_ptr, MAX_TEXT_SPRITES>& sprites)
{
    sprites.clear();
    print_line(gen, sprites, -72, "== CRC32 ==");

    uint8_t data[] = { 0x01, 0x02, 0x03, 0x04 };
    uint32_t crc_a = engine::save::crc32(data, 4);
    uint32_t crc_b = engine::save::crc32(data, 4);
    report(gen, sprites, -56, "Deterministic", crc_a == crc_b);

    uint8_t data2[] = { 0x05, 0x06, 0x07, 0x08 };
    uint32_t crc_c = engine::save::crc32(data2, 4);
    report(gen, sprites, -44, "Diff data", crc_a != crc_c);

    engine::save::SaveHeader good;
    good.magic = engine::save::SAVE_MAGIC;
    good.version = engine::save::SAVE_VERSION;
    report(gen, sprites, -32, "Header OK", engine::save::validate_header(good));

    engine::save::SaveHeader bad;
    bad.magic = 0xDEADBEEF;
    bad.version = 1;
    report(gen, sprites, -20, "Bad magic", !engine::save::validate_header(bad));

    good.checksum = crc_a;
    report(gen, sprites, -8, "Chksum match",
           engine::save::validate_checksum(good, data, 4));

    good.checksum = 0x12345678;
    report(gen, sprites, 4, "Chksum fail",
           !engine::save::validate_checksum(good, data, 4));

    print_line(gen, sprites, 28, "A = next");
    wait_for_a();
}

// ========================================
// TEST SUITE 2: Save & Load Roundtrip
// ========================================
void test_save_load(bn::sprite_text_generator& gen,
                    bn::vector<bn::sprite_ptr, MAX_TEXT_SPRITES>& sprites)
{
    sprites.clear();
    print_line(gen, sprites, -72, "== Save/Load ==");

    save_mgr.erase(0);
    save_mgr.erase(1);
    save_mgr.erase(2);

    TestState dummy;
    auto result = save_mgr.load(0, dummy);
    report(gen, sprites, -56, "Empty slot",
           result == engine::save::SaveResult::SLOT_EMPTY);

    report(gen, sprites, -44, "used=false",
           !save_mgr.is_slot_used(0));

    TestState to_save;
    to_save.player_x = 42;
    to_save.player_y = 73;
    to_save.level = 5;
    to_save.score = 9001;
    to_save.flags = 0xABCD1234;

    result = save_mgr.save(0, to_save);
    report(gen, sprites, -32, "Save OK",
           result == engine::save::SaveResult::OK);

    report(gen, sprites, -20, "used=true",
           save_mgr.is_slot_used(0));

    TestState loaded;
    result = save_mgr.load(0, loaded);
    bool data_ok = (result == engine::save::SaveResult::OK)
                   && (loaded.player_x == 42)
                   && (loaded.player_y == 73)
                   && (loaded.level == 5)
                   && (loaded.score == 9001)
                   && (loaded.flags == 0xABCD1234);
    report(gen, sprites, -8, "Roundtrip", data_ok);

    save_mgr.erase(0);
    report(gen, sprites, 4, "Erase", !save_mgr.is_slot_used(0));

    print_line(gen, sprites, 28, "A = next");
    wait_for_a();
}

// ========================================
// TEST SUITE 3: Multiple Slots
// ========================================
void test_multiple_slots(bn::sprite_text_generator& gen,
                         bn::vector<bn::sprite_ptr, MAX_TEXT_SPRITES>& sprites)
{
    sprites.clear();
    print_line(gen, sprites, -72, "== Slots ==");

    save_mgr.erase(0);
    save_mgr.erase(1);
    save_mgr.erase(2);

    TestState s0, s1, s2;
    s0.score = 100; s0.level = 1; s0.player_x = 10; s0.player_y = 10; s0.flags = 0;
    s1.score = 200; s1.level = 2; s1.player_x = 20; s1.player_y = 20; s1.flags = 0;
    s2.score = 300; s2.level = 3; s2.player_x = 30; s2.player_y = 30; s2.flags = 0;

    save_mgr.save(0, s0);
    save_mgr.save(1, s1);
    save_mgr.save(2, s2);

    report(gen, sprites, -56, "3 used",
           save_mgr.is_slot_used(0) &&
           save_mgr.is_slot_used(1) &&
           save_mgr.is_slot_used(2));

    TestState l0, l1, l2;
    save_mgr.load(0, l0);
    save_mgr.load(1, l1);
    save_mgr.load(2, l2);

    report(gen, sprites, -44, "S0=100", l0.score == 100);
    report(gen, sprites, -32, "S1=200", l1.score == 200);
    report(gen, sprites, -20, "S2=300", l2.score == 300);

    save_mgr.erase(1);
    report(gen, sprites, -8, "S1 erased", !save_mgr.is_slot_used(1));
    report(gen, sprites, 4, "S0 intact", save_mgr.is_slot_used(0));
    report(gen, sprites, 16, "S2 intact", save_mgr.is_slot_used(2));

    auto r = save_mgr.save(99, s0);
    report(gen, sprites, 28, "Bad idx",
           r == engine::save::SaveResult::SLOT_INVALID);

    print_line(gen, sprites, 48, "A = next");
    wait_for_a();
}

// ========================================
// TEST SUITE 4: Edge Cases
// ========================================
void test_edge_cases(bn::sprite_text_generator& gen,
                     bn::vector<bn::sprite_ptr, MAX_TEXT_SPRITES>& sprites)
{
    sprites.clear();
    print_line(gen, sprites, -72, "== Edge ==");

    TestState original;
    original.score = 500; original.level = 1;
    original.player_x = 0; original.player_y = 0; original.flags = 0;
    save_mgr.save(0, original);

    TestState updated;
    updated.score = 999; updated.level = 7;
    updated.player_x = 100; updated.player_y = 50; updated.flags = 0xFF;
    save_mgr.save(0, updated);

    TestState loaded;
    save_mgr.load(0, loaded);
    report(gen, sprites, -56, "Overwrite",
           loaded.score == 999 && loaded.level == 7);

    save_mgr.save(2, original);
    save_mgr.erase(2);
    auto r = save_mgr.load(2, loaded);
    report(gen, sprites, -44, "Erase+Load",
           r == engine::save::SaveResult::SLOT_EMPTY);

    r = save_mgr.load(-1, loaded);
    report(gen, sprites, -32, "Neg idx",
           r == engine::save::SaveResult::SLOT_INVALID);

    report(gen, sprites, -20, "cnt==3",
           save_mgr.slot_count() == 3);

    TestState precise;
    precise.player_x = bn::fixed(123.456);
    precise.player_y = bn::fixed(-78.9);
    precise.level = 0; precise.score = 0; precise.flags = 0;
    save_mgr.save(1, precise);

    TestState pl;
    save_mgr.load(1, pl);
    report(gen, sprites, -8, "bn::fixed OK",
           pl.player_x == precise.player_x &&
           pl.player_y == precise.player_y);

    print_line(gen, sprites, 16, "A = results");
    wait_for_a();
}

// ========================================
// RESULTS
// ========================================
void show_results(bn::sprite_text_generator& gen,
                  bn::vector<bn::sprite_ptr, MAX_TEXT_SPRITES>& sprites)
{
    sprites.clear();
    print_line(gen, sprites, -40, "== RESULTS ==");

    bn::string<32> p;
    p.append("Pass: ");
    p.append(bn::to_string<8>(tests_passed));
    print_line(gen, sprites, -20, p.c_str());

    bn::string<32> f;
    f.append("Fail: ");
    f.append(bn::to_string<8>(tests_failed));
    print_line(gen, sprites, -8, f.c_str());

    if(tests_failed == 0)
    {
        print_line(gen, sprites, 16, "ALL PASSED");
    }
    else
    {
        print_line(gen, sprites, 16, "SOME FAILED");
    }

    print_line(gen, sprites, 40, "START = rerun");
}

// ========================================
// MAIN
// ========================================
int main()
{
    bn::core::init();

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    bn::bg_palettes::set_transparent_color(bn::color(0, 0, 0));

    bn::vector<bn::sprite_ptr, MAX_TEXT_SPRITES> text_sprites;

    // --- Persistence check runs FIRST on every boot ---
    test_persistence_check(text_generator, text_sprites);

    while(true)
    {
        tests_passed = 0;
        tests_failed = 0;

        test_integrity(text_generator, text_sprites);
        test_save_load(text_generator, text_sprites);
        test_multiple_slots(text_generator, text_sprites);
        test_edge_cases(text_generator, text_sprites);
        show_results(text_generator, text_sprites);

        while(! bn::keypad::start_pressed())
        {
            bn::core::update();
        }
        bn::core::update();
    }
}