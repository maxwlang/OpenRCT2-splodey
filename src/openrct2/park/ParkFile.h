/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

struct ObjectRepositoryItem;

namespace OpenRCT2
{
    struct GameState_t;

    // Current version that is saved.
    constexpr uint32_t kParkFileCurrentVersion = 55;

    // The minimum version that is forwards compatible with the current version.
    constexpr uint32_t kParkFileMinVersion = 55;

    // The minimum version that is backwards compatible with the current version.
    // If this is increased beyond 0, uncomment the checks in ParkFile.cpp and Context.cpp!
    constexpr uint32_t kParkFileMinSupportedVersion = 0x0;

    constexpr uint32_t kParkFileMagic = 0x4B524150; // PARK

    struct IStream;

    // As uint16_t, in order to allow comparison with int32_t
    constexpr uint16_t kInversionsHolesShelteredEightsSplit = 6;
    constexpr uint16_t kLogFlumeSteepSlopeVersion = 16;
    constexpr uint16_t kBlockBrakeImprovementsVersion = 27;
    constexpr uint16_t kGigaCoasterInversions = 31;
    constexpr uint16_t kWoodenFlatToSteepVersion = 37;
    constexpr uint16_t k16BitParkHistoryVersion = 38;
    constexpr uint16_t kPeepNamesObjectsVersion = 39;
    constexpr uint16_t kWoodenRollerCoasterMediumLargeHalfLoopsVersion = 41;
    constexpr uint16_t kExtendedCorkscrewCoasterVersion = 42;
    constexpr uint16_t kExtendedTwisterCoasterVersion = 43;
    constexpr uint16_t kExtendedBoatHireVersion = 46;
    constexpr uint16_t kParkEntranceObjectLimitIncreased = 47;
    constexpr uint16_t kExtendedStandUpRollerCoasterVersion = 48;
    constexpr uint16_t kPeepAnimationObjectsVersion = 49;
    constexpr uint16_t kDiagonalLongFlatToSteepAndDiveLoopVersion = 50;
    constexpr uint16_t kEmbeddedParkPreviewChunk = 52;
    constexpr uint16_t kClimateObjectsVersion = 53;
    constexpr uint16_t kExtendedGoKartsVersion = 54;
    constexpr uint16_t kHigherInversionsHolesHelicesStatsVersion = 55;
} // namespace OpenRCT2

class ParkFileExporter
{
public:
    std::vector<const ObjectRepositoryItem*> ExportObjectsList;

    void Export(OpenRCT2::GameState_t& gameState, std::string_view path);
    void Export(OpenRCT2::GameState_t& gameState, OpenRCT2::IStream& stream);
};
