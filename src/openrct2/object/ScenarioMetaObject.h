/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "../core/IStream.hpp"
#include "../park/ParkPreview.h"
#include "Object.h"

#include <string>

class ScenarioMetaObject final : public Object
{
private:
    ImageIndex _imageOffsetId;

public:
    static constexpr ObjectType kObjectType = ObjectType::scenarioMeta;

    void ReadJson(IReadObjectContext* context, json_t& root) override;
    void Load() override;
    void Unload() override;

    std::string GetScenarioName();
    std::string GetParkName();
    std::string GetScenarioDetails();

    OpenRCT2::PreviewImage GetMiniMapImage() const;
    OpenRCT2::PreviewImage GetPreviewImage() const;
};
