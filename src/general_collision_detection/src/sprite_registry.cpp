#include "sprite_registry.h"
#include "sprite.h"

void SpriteRegistry::sync_all(const Camera& camera) {
    for(int i = 0; i < _count; ++i) {
        _sprites[i]->sync(camera);
    }
}
