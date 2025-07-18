/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Fountain.h"

#include "../Game.h"
#include "../GameState.h"
#include "../core/DataSerialiser.h"
#include "../object/PathAdditionEntry.h"
#include "../paint/Paint.h"
#include "../profiling/Profiling.h"
#include "../world/Footpath.h"
#include "../world/Location.hpp"
#include "../world/Map.h"
#include "../world/Scenery.h"
#include "../world/tile_element/PathElement.h"
#include "EntityRegistry.h"

using namespace OpenRCT2;

enum class Pattern
{
    cyclicSquares,
    continuousChasers,
    bouncingPairs,
    sproutingBlooms,
    racingPairs,
    splittingChasers,
    dopeyJumpers,
    fastRandomChasers,
};

static constexpr std::array _fountainDirectionsNegative = {
    CoordsXY{ -kCoordsXYStep, 0 },
    CoordsXY{ -kCoordsXYStep, -kCoordsXYStep },
    CoordsXY{ 0, 0 },
    CoordsXY{ -kCoordsXYStep, 0 },
    CoordsXY{ 0, 0 },
    CoordsXY{ 0, -kCoordsXYStep },
    CoordsXY{ 0, -kCoordsXYStep },
    CoordsXY{ -kCoordsXYStep, -kCoordsXYStep },
};

static constexpr std::array _fountainDirectionsPositive = {
    CoordsXY{ kCoordsXYStep, 0 },
    CoordsXY{ 0, 0 },
    CoordsXY{ 0, kCoordsXYStep },
    CoordsXY{ kCoordsXYStep, kCoordsXYStep },
    CoordsXY{ kCoordsXYStep, kCoordsXYStep },
    CoordsXY{ kCoordsXYStep, 0 },
    CoordsXY{ 0, 0 },
    CoordsXY{ 0, kCoordsXYStep },
};
static constexpr auto kFountainChanceOfStoppingEdgeMode = 0x3333;   // 0.200
static constexpr auto kFountainChanceOfStoppingRandomMode = 0x2000; // 0.125

// rct2: 0x0097F040
const uint8_t _fountainDirections[] = {
    0, 1, 2, 3, 0, 1, 2, 3,
};

// rct2: 0x0097F048
const FountainFlags _fountainDirectionFlags[] = {
    {}, {}, FountainFlag::direction, FountainFlag::direction, FountainFlag::direction, FountainFlag::direction, {}, {},
};

// rct2: 0x0097F050
const FountainFlags _fountainPatternFlags[] = {
    { FountainFlag::terminate },                                         // cyclicSquares
    { FountainFlag::fast, FountainFlag::goToEdge },                      // continuousChasers
    { FountainFlag::bounce },                                            // bouncingPairs
    { FountainFlag::fast, FountainFlag::split },                         // sproutingBlooms
    { FountainFlag::goToEdge },                                          // racingPairs
    { FountainFlag::fast, FountainFlag::goToEdge, FountainFlag::split }, // splittingChasers
    {},                                                                  // dopeyJumpers
    { FountainFlag::fast },                                              // fastRandomChasers
};

template<>
bool EntityBase::Is<JumpingFountain>() const
{
    return Type == EntityType::JumpingFountain;
}

void JumpingFountain::StartAnimation(const JumpingFountainType newType, const CoordsXY& newLoc, const TileElement* tileElement)
{
    const auto currentTicks = getGameState().currentTicks;

    int32_t randomIndex;
    auto newZ = tileElement->GetBaseZ();

    // Change pattern approximately every 51 seconds
    uint32_t pattern = (currentTicks >> 11) & 7;
    switch (static_cast<Pattern>(pattern))
    {
        case Pattern::cyclicSquares:
            // 0, 1, 2, 3
            for (int32_t i = 0; i < kNumOrthogonalDirections; i++)
            {
                JumpingFountain::Create(
                    newType, { newLoc + _fountainDirectionsPositive[i], newZ }, _fountainDirections[i],
                    _fountainDirectionFlags[i] | _fountainPatternFlags[pattern], 0);
            }
            break;
        case Pattern::bouncingPairs:
            // random [0, 2 or 1, 3]
            randomIndex = ScenarioRand() & 1;
            for (int32_t i = randomIndex; i < kNumOrthogonalDirections; i += 2)
            {
                JumpingFountain::Create(
                    newType, { newLoc + _fountainDirectionsPositive[i], newZ }, _fountainDirections[i],
                    _fountainDirectionFlags[i] | _fountainPatternFlags[pattern], 0);
            }
            break;
        case Pattern::racingPairs:
            // random [0 - 3 and 4 - 7]
            randomIndex = ScenarioRand() & 3;
            JumpingFountain::Create(
                newType, { newLoc + _fountainDirectionsPositive[randomIndex], newZ }, _fountainDirections[randomIndex],
                _fountainDirectionFlags[randomIndex] | _fountainPatternFlags[pattern], 0);
            randomIndex += 4;
            JumpingFountain::Create(
                newType, { newLoc + _fountainDirectionsPositive[randomIndex], newZ }, _fountainDirections[randomIndex],
                _fountainDirectionFlags[randomIndex] | _fountainPatternFlags[pattern], 0);
            break;
        default:
            // random [0 - 7]
            randomIndex = ScenarioRand() & 7;
            JumpingFountain::Create(
                newType, { newLoc + _fountainDirectionsPositive[randomIndex], newZ }, _fountainDirections[randomIndex],
                _fountainDirectionFlags[randomIndex] | _fountainPatternFlags[pattern], 0);
            break;
    }
}

void JumpingFountain::Create(
    const JumpingFountainType newType, const CoordsXYZ& newLoc, const int32_t direction, const OpenRCT2::FountainFlags newFlags,
    const int32_t iteration)
{
    auto* jumpingFountain = CreateEntity<JumpingFountain>();
    if (jumpingFountain != nullptr)
    {
        jumpingFountain->Iteration = iteration;
        jumpingFountain->fountainFlags = newFlags;
        jumpingFountain->Orientation = direction << 3;
        jumpingFountain->SpriteData.Width = 33;
        jumpingFountain->SpriteData.HeightMin = 36;
        jumpingFountain->SpriteData.HeightMax = 12;
        jumpingFountain->MoveTo(newLoc);
        jumpingFountain->FountainType = newType;
        jumpingFountain->NumTicksAlive = 0;
        jumpingFountain->frame = 0;
    }
}

void JumpingFountain::Update()
{
    NumTicksAlive++;
    // Originally this would not update the frame on the following
    // ticks: 1, 3, 6, 9, 11, 14, 17, 19, 22, 25
    // This change was to simplify the code base. There is a small increase
    // in speed of the fountain jump because of this change.
    if (NumTicksAlive % 3 == 0)
    {
        return;
    }

    Invalidate();
    frame++;

    switch (FountainType)
    {
        case JumpingFountainType::Water:
            if (frame == 11 && (fountainFlags.has(FountainFlag::fast)))
            {
                AdvanceAnimation();
            }
            if (frame == 16 && !(fountainFlags.has(FountainFlag::fast)))
            {
                AdvanceAnimation();
            }
            break;
        case JumpingFountainType::Snow:
            if (frame == 16)
            {
                AdvanceAnimation();
            }
            break;
        default:
            break;
    }

    if (frame == 16)
    {
        EntityRemove(this);
    }
}

JumpingFountainType JumpingFountain::GetType() const
{
    return FountainType;
}

void JumpingFountain::AdvanceAnimation()
{
    const JumpingFountainType newType = GetType();
    const int32_t direction = (Orientation >> 3) & 7;
    const CoordsXY newLoc = CoordsXY{ x, y } + CoordsDirectionDelta[direction];

    int32_t availableDirections = 0;
    for (uint32_t i = 0; i < _fountainDirectionsNegative.size(); i++)
    {
        if (IsJumpingFountain(newType, { newLoc + _fountainDirectionsNegative[i], z }))
        {
            availableDirections |= 1 << i;
        }
    }

    if (availableDirections == 0)
    {
        return;
    }

    if (fountainFlags.has(FountainFlag::terminate))
    {
        return;
    }

    if (fountainFlags.has(FountainFlag::goToEdge))
    {
        GoToEdge({ newLoc, z }, availableDirections);
        return;
    }

    if (fountainFlags.has(FountainFlag::bounce))
    {
        Bounce({ newLoc, z }, availableDirections);
        return;
    }

    if (fountainFlags.has(FountainFlag::split))
    {
        Split({ newLoc, z }, availableDirections);
        return;
    }

    Random({ newLoc, z }, availableDirections);
}

bool JumpingFountain::IsJumpingFountain(const JumpingFountainType newType, const CoordsXYZ& newLoc)
{
    const int32_t pathAdditionFlagMask = newType == JumpingFountainType::Snow ? PATH_ADDITION_FLAG_JUMPING_FOUNTAIN_SNOW
                                                                              : PATH_ADDITION_FLAG_JUMPING_FOUNTAIN_WATER;

    TileElement* tileElement = MapGetFirstElementAt(newLoc);
    if (tileElement == nullptr)
        return false;
    do
    {
        if (tileElement->GetType() != TileElementType::Path)
            continue;
        if (tileElement->GetBaseZ() != newLoc.z)
            continue;
        if (tileElement->AsPath()->AdditionIsGhost())
            continue;
        if (!tileElement->AsPath()->HasAddition())
            continue;

        auto* pathAdditionEntry = tileElement->AsPath()->GetAdditionEntry();
        if (pathAdditionEntry != nullptr && pathAdditionEntry->flags & pathAdditionFlagMask)
        {
            return true;
        }
    } while (!(tileElement++)->IsLastForTile());

    return false;
}

void JumpingFountain::GoToEdge(const CoordsXYZ& newLoc, const int32_t availableDirections) const
{
    int32_t direction = (Orientation >> 3) << 1;
    if (availableDirections & (1 << direction))
    {
        CreateNext(newLoc, direction);
        return;
    }

    direction++;
    if (availableDirections & (1 << direction))
    {
        CreateNext(newLoc, direction);
        return;
    }

    const uint32_t randomIndex = ScenarioRand();
    if ((randomIndex & 0xFFFF) < kFountainChanceOfStoppingEdgeMode)
    {
        return;
    }

    if (fountainFlags.has(FountainFlag::split))
    {
        Split(newLoc, availableDirections);
        return;
    }

    direction = randomIndex & 7;
    while (!(availableDirections & (1 << direction)))
    {
        direction = (direction + 1) & 7;
    }

    CreateNext(newLoc, direction);
}

void JumpingFountain::Bounce(const CoordsXYZ& newLoc, const int32_t availableDirections)
{
    Iteration++;
    if (Iteration < 8)
    {
        int32_t direction = ((Orientation >> 3) ^ 2) << 1;
        if (availableDirections & (1 << direction))
        {
            CreateNext(newLoc, direction);
        }
        else
        {
            direction++;
            if (availableDirections & (1 << direction))
            {
                CreateNext(newLoc, direction);
            }
        }
    }
}

void JumpingFountain::Split(const CoordsXYZ& newLoc, int32_t availableDirections) const
{
    if (Iteration < 3)
    {
        const auto newType = GetType();
        int32_t direction = ((Orientation >> 3) ^ 2) << 1;
        availableDirections &= ~(1 << direction);
        availableDirections &= ~(1 << (direction + 1));

        for (direction = 0; direction < 8; direction++)
        {
            if (availableDirections & (1 << direction))
            {
                auto copiedFlags = fountainFlags;
                copiedFlags.unset(FountainFlag::direction);
                JumpingFountain::Create(newType, newLoc, direction >> 1, copiedFlags, Iteration + 1);
            }
            direction++;
            if (availableDirections & (1 << direction))
            {
                auto copiedFlags = fountainFlags;
                copiedFlags.set(FountainFlag::direction);
                JumpingFountain::Create(newType, newLoc, direction >> 1, copiedFlags, Iteration + 1);
            }
        }
    }
}

void JumpingFountain::Random(const CoordsXYZ& newLoc, int32_t availableDirections) const
{
    const uint32_t randomIndex = ScenarioRand();
    if ((randomIndex & 0xFFFF) >= kFountainChanceOfStoppingRandomMode)
    {
        int32_t direction = randomIndex & 7;
        while (!(availableDirections & (1 << direction)))
        {
            direction = (direction + 1) & 7;
        }
        CreateNext(newLoc, direction);
    }
}

void JumpingFountain::CreateNext(const CoordsXYZ& newLoc, int32_t direction) const
{
    const auto newType = GetType();
    auto newFlags = fountainFlags;
    newFlags.set(FountainFlag::direction, !!(direction & 1));
    JumpingFountain::Create(newType, newLoc, direction >> 1, newFlags, Iteration);
}

void JumpingFountain::Serialise(DataSerialiser& stream)
{
    EntityBase::Serialise(stream);
    stream << frame;
    stream << FountainType;
    stream << NumTicksAlive;
    stream << fountainFlags.holder;
    stream << TargetX;
    stream << TargetY;
    stream << Iteration;
}

void JumpingFountain::Paint(PaintSession& session, int32_t imageDirection) const
{
    PROFILED_FUNCTION();

    // TODO: Move into SpriteIds.h
    constexpr uint32_t kJumpingFountainSnowBaseImage = 23037;
    constexpr uint32_t kJumpingFountainWaterBaseImage = 22973;

    RenderTarget& rt = session.DPI;
    if (rt.zoom_level > ZoomLevel{ 0 })
    {
        return;
    }

    uint16_t height = z + 6;
    imageDirection = imageDirection / 8;

    // Fountain is firing anti clockwise
    bool reversed = fountainFlags.has(FountainFlag::direction);
    // Fountain rotation
    bool rotated = (Orientation / 16) & 1;
    bool isAntiClockwise = (imageDirection / 2) & 1; // Clockwise or Anti-clockwise

    // These cancel each other out
    if (reversed != rotated)
    {
        isAntiClockwise = !isAntiClockwise;
    }

    uint32_t baseImageId = (FountainType == JumpingFountainType::Snow) ? kJumpingFountainSnowBaseImage
                                                                       : kJumpingFountainWaterBaseImage;
    auto imageId = ImageId(baseImageId + imageDirection * 16 + frame);
    constexpr std::array antiClockWiseBoundingBoxes = {
        CoordsXY{ -kCoordsXYStep, -3 },
        CoordsXY{ 0, -3 },
    };
    constexpr std::array clockWiseBoundingBoxes = {
        CoordsXY{ -kCoordsXYStep, 3 },
        CoordsXY{ 0, 3 },
    };

    auto bb = isAntiClockwise ? antiClockWiseBoundingBoxes : clockWiseBoundingBoxes;

    PaintAddImageAsParentRotated(
        session, imageDirection, imageId, { 0, 0, height }, { { bb[imageDirection & 1], height }, { 32, 1, 3 } });
}
