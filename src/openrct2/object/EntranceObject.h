/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "../world/Entrance.h"
#include "../world/Location.hpp"
#include "EntranceEntry.h"
#include "Object.h"

class EntranceObject final : public Object
{
private:
    EntranceEntry _legacyType = {};

public:
    static constexpr ObjectType kObjectType = ObjectType::parkEntrance;

    void* GetLegacyData() override
    {
        return &_legacyType;
    }

    void ReadLegacy(IReadObjectContext* context, OpenRCT2::IStream* stream) override;
    void ReadJson(IReadObjectContext* context, json_t& root) override;
    void Load() override;
    void Unload() override;

    void DrawPreview(RenderTarget& rt, int32_t width, int32_t height) const override;

    ImageIndex GetImage(uint8_t sequence, Direction direction) const;
    uint8_t GetScrollingMode() const;
    uint8_t GetTextHeight() const;
};
