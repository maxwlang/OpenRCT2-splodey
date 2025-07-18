/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "Location.hpp"

namespace OpenRCT2::MapAnimations
{
    enum class TemporaryType : uint8_t
    {
        onRidePhoto,
        landEdgeDoor,
    };

    void MarkTileForInvalidation(const TileCoordsXY coords);
    void MarkTileForUpdate(const TileCoordsXY coords);
    void CreateTemporary(const CoordsXYZ& coords, const TemporaryType type);
    void MarkAllTiles();
    void InvalidateAndUpdateAll();
    void ClearAll();
    void ShiftAll(const TileCoordsXY amount);
} // namespace OpenRCT2::MapAnimations
