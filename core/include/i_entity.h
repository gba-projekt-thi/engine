#pragma once

// -----------------------------------------------------------------------------
// IEntity
//
// Extension point for future entity types that need a per-frame update()
// but are NOT already a CollisionShape (traps/triggers/door/player already
// get free polymorphic update() dispatch through CollisionRegistry - see
// CollisionSystem.md - so this interface intentionally does not cover
// them, to avoid two unrelated virtual update() overloads colliding on the
// same derived class).
//
// Intended for purely visual/audio entities with no collision footprint,
// e.g. animated decorative background props, parallax layers, or particle
// effects - none of which exist in the game yet, so this interface is NOT
// wired into LevelManager until a concrete use case needs it. When one
// does: add a `bn::vector<IEntity*, N> _visual_entities` to LevelManager,
// push_back() from the relevant _load_*() stage, call update() on each
// during LevelManager::update()'s render step.
//
// A type needing both a per-frame update and a level-reset hook (e.g. an
// animated prop that should replay from frame 0 after death) can inherit
// both IEntity and IResettable - they compose independently.
// -----------------------------------------------------------------------------
class IEntity {
   public:
    virtual void update() = 0;

   protected:
    ~IEntity() = default;
};