/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "../audio/Audio.h"
#include "Object.h"

class TerrainEdgeObject final : public Object
{
private:
public:
    static constexpr ObjectType kObjectType = ObjectType::terrainEdge;

    StringId NameStringId{};
    uint32_t IconImageId{};
    uint32_t BaseImageId{};
    bool HasDoors{};
    OpenRCT2::Audio::DoorSoundType doorSound{};

    void ReadJson(IReadObjectContext* context, json_t& root) override;
    void Load() override;
    void Unload() override;

    void DrawPreview(RenderTarget& rt, int32_t width, int32_t height) const override;

    static TerrainEdgeObject* GetById(ObjectEntryIndex entryIndex);
};
