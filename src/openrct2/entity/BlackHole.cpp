#include "BlackHole.h"

#include "EntityList.h"
#include "EntityRegistry.h"
#include "Guest.h"
#include "Staff.h"
#include "../world/Map.h"
#include "../paint/Paint.h"

BlackHole* BlackHole::Create(const CoordsXYZ& loc)
{
    auto* bh = CreateEntity<BlackHole>();
    if (bh != nullptr)
    {
        bh->SpriteData.Width = 16;
        bh->SpriteData.HeightMin = 16;
        bh->SpriteData.HeightMax = 16;
        bh->MoveTo(loc);
    }
    return bh;
}

void BlackHole::Update()
{
    for (auto guest : EntityList<Guest>())
    {
        int32_t dx = x - guest->x;
        int32_t dy = y - guest->y;
        guest->MoveTo({ guest->x + dx / 16, guest->y + dy / 16, guest->z });
    }
    for (auto staff : EntityList<Staff>())
    {
        int32_t dx = x - staff->x;
        int32_t dy = y - staff->y;
        staff->MoveTo({ staff->x + dx / 16, staff->y + dy / 16, staff->z });
    }
}

void BlackHole::Serialise(DataSerialiser& stream)
{
    EntityBase::Serialise(stream);
}

void BlackHole::Paint(PaintSession&, int32_t) const
{
    // No drawing
}
