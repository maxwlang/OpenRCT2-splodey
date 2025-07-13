#pragma once

#include "EntityBase.h"

struct BlackHole : EntityBase
{
    static constexpr auto cEntityType = EntityType::BlackHole;

    static BlackHole* Create(const CoordsXYZ& loc);
    void Update();
    void Serialise(DataSerialiser& stream);
    void Paint(PaintSession& session, int32_t imageDirection) const;
};
