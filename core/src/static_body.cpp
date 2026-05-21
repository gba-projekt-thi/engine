#include "static_body.h"

StaticBody::StaticBody(bn::fixed x, bn::fixed y, bn::fixed w, bn::fixed h,
                       uint16_t layers_)
    : pos{x, y, w, h}, layers(layers_) {
    CollisionRegistry::instance().register_body(this);
}

StaticBody::~StaticBody() {
    CollisionRegistry::instance().unregister_body(this);
}
