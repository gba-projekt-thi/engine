#include "collision_registry.h"
#include "static_body.h"
#include "physics_body.h"

CollisionRegistry& CollisionRegistry::instance() {
    static CollisionRegistry reg;
    return reg;
}

void CollisionRegistry::register_body(StaticBody* b) {
    if(_count < MAX_BODIES) {
        _bodies[_count] = b;
        ++_count;
    }
}

void CollisionRegistry::unregister_body(StaticBody* b) {
    for(int i = 0; i < _count; ++i) {
        if(_bodies[i] == b) {
            _bodies[i] = _bodies[_count - 1];
            --_count;
            return;
        }
    }
}

void CollisionRegistry::update_all() {
    for(int i = 0; i < _count; ++i) {
        if(_bodies[i]->needs_physics_update) {
            static_cast<PhysicsBody*>(_bodies[i])->physics_update();
        }
    }
}