#pragma once

#include "physics_body.h"
#include "sprite.h"
#include "enemy.h"
#include "bn_keypad.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_color.h"

class Player : public PhysicsBody {

public:
    static constexpr uint8_t  BODY_TYPE   = 1;         // for casting in on_enter
    static constexpr uint16_t LAYERS      = 0x0001;   // what am I
    static constexpr uint16_t ENEMY_LAYER = 0x0002;
    static constexpr uint16_t DOOR_LAYER  = 0x0004;
    static constexpr uint16_t TILEMAP     = 0x8000;

    static constexpr uint16_t MASK  = ENEMY_LAYER | DOOR_LAYER;                // what I detect  (on_enter / on_exit)
    static constexpr uint16_t BLOCK = 0xFFFF & ~ENEMY_LAYER & ~DOOR_LAYER;     // what blocks me (pass through enemies and doors)

    // tweak these to taste
    bn::fixed gravity        = bn::fixed(0.15);
    bn::fixed move_speed     = bn::fixed(0.3);
    bn::fixed jump_force     = bn::fixed(5);
    bn::fixed wall_jump_push = bn::fixed(2);
    bn::fixed friction       = bn::fixed(0.15);

    // Door Variables
    bool on_door = false;
    bn::fixed spawn_x;
    bn::fixed spawn_y;

    Player(bn::fixed start_x, bn::fixed start_y, bn::fixed w, bn::fixed h)
        : PhysicsBody(start_x, start_y, w, h, LAYERS, MASK, BLOCK),
          spawn_x(start_x), spawn_y(start_y) {
        body_type = BODY_TYPE;
        vel_max = 4;
    }

    void update() override {
        // gravity
        inc_velocity(0, gravity);

        // horizontal
        if(bn::keypad::left_held()) {
            inc_velocity(-move_speed, 0);
        } else if(bn::keypad::right_held()) {
            inc_velocity(move_speed, 0);
        } else {
            dec_velocity(friction, 0);
        }

        bool grounded = probe_bottom().combined_layers() & TILEMAP;

        if(bn::keypad::a_pressed()) {
            if(grounded) {
                set_velocity(vel_x, -jump_force);
            } else if(probe_left().combined_layers()) {
                set_velocity(wall_jump_push, -jump_force);
            } else if(probe_right().combined_layers()) {
                set_velocity(-wall_jump_push, -jump_force);
            }
        }

        // Door Logic
        if(on_door && bn::keypad::b_pressed()) {
            pos.x = spawn_x;
            pos.y = spawn_y;
            set_velocity(0, 0);
        }

        // test on/off: hold L to disable collision shape
        if(bn::keypad::l_held()){
            shape_disable();
            disable();
        } else {
            shape_enable();
            enable();
        }
    }

    void on_enter(uint16_t hit_layers, StaticBody* body) override {
        // 1. check layer
        if(hit_layers & ENEMY_LAYER) {
            // 2. check body is not nullptr (nullptr = map rect)
            if(body) {
                // 3. check body_type matches Enemy
                if(body->body_type == Enemy::BODY_TYPE) {
                    // 4. safe to cast
                    Enemy& enemy = static_cast<Enemy&>(*body);
                    // 5. access the enemy's attributes
                    if(enemy.sprite) {
                        Sprite* enemy_sprite = enemy.sprite;
                        bn::sprite_palette_ptr pal = enemy_sprite->sprite().palette();
                        pal.set_fade(bn::color(31, 31, 31), 1);
                    }
                }
            }
        }
        if(hit_layers & DOOR_LAYER) {
            on_door = true;
        }
    }

    void on_exit(uint16_t hit_layers, StaticBody* body) override {
        if((hit_layers & ENEMY_LAYER) && body && body->body_type == Enemy::BODY_TYPE) {
            Enemy& enemy = static_cast<Enemy&>(*body);
            if(enemy.sprite) {
                bn::sprite_palette_ptr pal = enemy.sprite->sprite().palette();
                pal.set_fade(bn::color(31, 31, 31), 0);
            }
        }
        if(hit_layers & DOOR_LAYER) {
            on_door = false;
        }
    }
};
