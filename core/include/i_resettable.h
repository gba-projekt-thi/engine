#pragma once

// -----------------------------------------------------------------------------
// IResettable
//
// Minimal interface for anything LevelManager needs to reset back to its
// level-start state after the player dies (or the level restarts). Only
// reset() is part of the contract - construction/teardown stays with each
// concrete type's own constructor/destructor, and per-frame behavior stays
// with whatever update mechanism the type already has (most collidable
// entities already get free polymorphic update() dispatch through
// CollisionShape - see CollisionSystem.md - so IResettable intentionally
// does not duplicate that).
//
// Storage:
// Implementations are referenced through non-owning raw pointers (see
// LevelManager::_resettables), so the destructor does not need to be
// virtual/public - mirrors the PlayerDeathHandler callback pattern.
// -----------------------------------------------------------------------------
class IResettable {
   public:
    virtual void reset() = 0;

   protected:
    ~IResettable() = default;
};