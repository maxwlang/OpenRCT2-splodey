/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "../UiStringIds.h"

#include <iterator>
#include <openrct2-ui/interface/Dropdown.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Windows.h>
#include <openrct2/Context.h>
#include <openrct2/Game.h>
#include <openrct2/GameState.h>
#include <openrct2/OpenRCT2.h>
#include <openrct2/SpriteIds.h>
#include <openrct2/actions/CheatSetAction.h>
#include <openrct2/actions/ParkSetDateAction.h>
#include <openrct2/core/EnumUtils.hpp>
#include <openrct2/localisation/Currency.h>
#include <openrct2/localisation/Formatter.h>
#include <openrct2/localisation/Localisation.Date.h>
#include <openrct2/network/Network.h>
#include <openrct2/ui/WindowManager.h>
#include <openrct2/util/Util.h>
#include <openrct2/world/Park.h>
#include <openrct2/world/tile_element/SurfaceElement.h>

namespace OpenRCT2::Ui::Windows
{
    constexpr auto kCheatsMoneyDefault = 10000.00_GBP;
    constexpr auto kCheatsMoneyIncrement = 5000.00_GBP;

    // clang-format off
enum
{
    WINDOW_CHEATS_PAGE_MONEY,
    WINDOW_CHEATS_PAGE_DATE,
    WINDOW_CHEATS_PAGE_GUESTS,
    WINDOW_CHEATS_PAGE_STAFF,
    WINDOW_CHEATS_PAGE_PARK,
    WINDOW_CHEATS_PAGE_RIDES,
    WINDOW_CHEATS_PAGE_WEATHER,
    WINDOW_CHEATS_PAGE_COUNT,
};

static StringId _staffSpeedNames[] =
{
    STR_NORMAL,
    STR_FROZEN,
    STR_FAST,
};

static constexpr StringId WeatherTypes[] =
{
    STR_SUNNY,
    STR_PARTIALLY_CLOUDY,
    STR_CLOUDY,
    STR_RAIN,
    STR_HEAVY_RAIN,
    STR_THUNDERSTORM,
    STR_SNOW,
    STR_HEAVY_SNOW,
    STR_BLIZZARD,
};

enum WindowCheatsWidgetIdx
{
    WIDX_BACKGROUND,
    WIDX_TITLE,
    WIDX_CLOSE,
    WIDX_PAGE_BACKGROUND,
    WIDX_TAB_1,
    WIDX_TAB_2,
    WIDX_TAB_3,
    WIDX_TAB_4,
    WIDX_TAB_5,
    WIDX_TAB_6,
    WIDX_TAB_7,
    WIDX_TAB_CONTENT,

    WIDX_NO_MONEY = WIDX_TAB_CONTENT,
    WIDX_ADD_SET_MONEY_GROUP,
    WIDX_MONEY_SPINNER,
    WIDX_MONEY_SPINNER_INCREMENT,
    WIDX_MONEY_SPINNER_DECREMENT,
    WIDX_ADD_MONEY,
    WIDX_SET_MONEY,
    WIDX_CLEAR_LOAN,

    WIDX_DATE_GROUP = WIDX_TAB_CONTENT,
    WIDX_YEAR_BOX,
    WIDX_YEAR_UP,
    WIDX_YEAR_DOWN,
    WIDX_MONTH_BOX,
    WIDX_MONTH_UP,
    WIDX_MONTH_DOWN,
    WIDX_DAY_BOX,
    WIDX_DAY_UP,
    WIDX_DAY_DOWN,
    WIDX_DATE_SET,
    WIDX_DATE_RESET,

    WIDX_TRAM_GUESTS = WIDX_TAB_CONTENT,
    WIDX_REMOVE_ALL_GUESTS,
    WIDX_GUEST_PARAMETERS_GROUP,
    WIDX_GUEST_HAPPINESS_MAX,
    WIDX_GUEST_HAPPINESS_MIN,
    WIDX_GUEST_ENERGY_MAX,
    WIDX_GUEST_ENERGY_MIN,
    WIDX_GUEST_HUNGER_MAX,
    WIDX_GUEST_HUNGER_MIN,
    WIDX_GUEST_THIRST_MAX,
    WIDX_GUEST_THIRST_MIN,
    WIDX_GUEST_NAUSEA_MAX,
    WIDX_GUEST_NAUSEA_MIN,
    WIDX_GUEST_NAUSEA_TOLERANCE_MAX,
    WIDX_GUEST_NAUSEA_TOLERANCE_MIN,
    WIDX_GUEST_TOILET_MAX,
    WIDX_GUEST_TOILET_MIN,
    WIDX_GUEST_RIDE_INTENSITY_MORE_THAN_1,
    WIDX_GUEST_RIDE_INTENSITY_LESS_THAN_15,
    WIDX_GIVE_ALL_GUESTS_GROUP,
    WIDX_GIVE_GUESTS_MONEY,
    WIDX_GIVE_GUESTS_PARK_MAPS,
    WIDX_GIVE_GUESTS_BALLOONS,
    WIDX_GIVE_GUESTS_UMBRELLAS,
    WIDX_GUEST_BEHAVIOUR_GROUP,
    WIDX_GUEST_IGNORE_RIDE_INTENSITY,
    WIDX_GUEST_IGNORE_PRICE,
    WIDX_DISABLE_VANDALISM,
    WIDX_DISABLE_LITTERING,

    WIDX_STAFF_GROUP = WIDX_TAB_CONTENT,
    WIDX_STAFF_SPEED,
    WIDX_STAFF_SPEED_DROPDOWN_BUTTON,
    WIDX_MAINTENANCE_GROUP,
    WIDX_REMOVE_LITTER,
    WIDX_FIX_VANDALISM,
    WIDX_CLEAR_GRASS,
    WIDX_MOWED_GRASS,
    WIDX_WATER_PLANTS,
    WIDX_DISABLE_PLANT_AGING,

    WIDX_GENERAL_GROUP = WIDX_TAB_CONTENT,
    WIDX_OWN_ALL_LAND,
    WIDX_REMOVE_PARK_FENCES,
    WIDX_OPEN_CLOSE_PARK,
    WIDX_OBJECTIVE_GROUP,
    WIDX_NEVERENDING_MARKETING,
    WIDX_FORCE_PARK_RATING,
    WIDX_PARK_RATING_SPINNER,
    WIDX_INCREASE_PARK_RATING,
    WIDX_DECREASE_PARK_RATING,
    WIDX_WIN_SCENARIO,
    WIDX_HAVE_FUN,
    WIDX_PARK_CONSTRUCTION_GROUP,
    WIDX_ALLOW_BUILD_IN_PAUSE_MODE,
    WIDX_ALLOW_REGULAR_PATH_AS_QUEUE,
    WIDX_ALLOW_SPECIAL_COLOUR_SCHEMES,

    WIDX_FIX_ALL = WIDX_TAB_CONTENT,
    WIDX_RENEW_RIDES,
    WIDX_RESET_CRASH_STATUS,
    WIDX_10_MINUTE_INSPECTIONS,
    WIDX_CONSTRUCTION_GROUP,
    WIDX_ENABLE_ALL_DRAWABLE_TRACK_PIECES,
    WIDX_ENABLE_CHAIN_LIFT_ON_ALL_TRACK,
    WIDX_ALLOW_TRACK_PLACE_INVALID_HEIGHTS,
    WIDX_MAKE_DESTRUCTIBLE,
    WIDX_OPERATION_MODES_GROUP,
    WIDX_SHOW_ALL_OPERATING_MODES,
    WIDX_UNLOCK_OPERATING_LIMITS,
    WIDX_DISABLE_BRAKES_FAILURE,
    WIDX_DISABLE_ALL_BREAKDOWNS,
    WIDX_DISABLE_RIDE_VALUE_AGING,
    WIDX_TRACK_PIECES_GROUP,
    WIDX_ENABLE_ARBITRARY_RIDE_TYPE_CHANGES,
    WIDX_SHOW_VEHICLES_FROM_OTHER_TRACK_TYPES,
    WIDX_DISABLE_TRAIN_LENGTH_LIMITS,
    WIDX_IGNORE_RESEARCH_STATUS,
    WIDX_ALLOW_INCOMPLETE_RIDES,
    WIDX_NORMALIZE_RIDE_CRASHES,

    WIDX_WEATHER_GROUP = WIDX_TAB_CONTENT,
    WIDX_WEATHER,
    WIDX_WEATHER_DROPDOWN_BUTTON,
    WIDX_FREEZE_WEATHER,
    WIDX_FAUNA_GROUP,
    WIDX_CREATE_DUCKS,
    WIDX_REMOVE_DUCKS,
};

#pragma region MEASUREMENTS

static constexpr StringId kWindowTitle = STR_CHEAT_TITLE;
static constexpr ScreenSize kWindowSize = { 249, 300 };

static constexpr ScreenSize kCheatButtonSize = {110, 17};
static constexpr ScreenSize kCheatCheckSize = {221, 12};
static constexpr ScreenSize kCheatSpinnerSize = {117, 14};
static constexpr ScreenSize kMinMaxButtonSize = {55, 17};

static constexpr int32_t kTabWidth = 31;
static constexpr int32_t kTabStart = 3;

#pragma endregion

static constexpr auto kMainCheatWidgets = makeWidgets(
    makeWindowShim(kWindowTitle, kWindowSize),
    makeWidget({  0, 43}, {kWindowSize.width, 257}, WidgetType::resize, WindowColour::secondary), /* tab content panel */
    makeTab   ({  3, 17}, STR_FINANCIAL_CHEATS_TIP                      ), /* tab 1 */
    makeTab   ({ 34, 17}, STR_DATE_CHEATS_TIP                           ), /* tab 2 */
    makeTab   ({ 65, 17}, STR_GUEST_CHEATS_TIP                          ), /* tab 3 */
    makeTab   ({ 96, 17}, STR_STAFF_CHEATS_TIP                          ), /* tab 5 */
    makeTab   ({127, 17}, STR_PARK_CHEATS_TIP                           ), /* tab 6 */
    makeTab   ({158, 17}, STR_RIDE_CHEATS_TIP                           ), /* tab 4 */
    makeTab   ({189, 17}, STR_WEATHER_NATURE_CHEATS_TIP                 )  /* tab 7 */
);

static constexpr auto window_cheats_money_widgets = makeWidgets(
    kMainCheatWidgets,
    makeWidget        ({ 11,  48}, kCheatButtonSize,  WidgetType::checkbox, WindowColour::secondary, STR_MAKE_PARK_NO_MONEY), // No money
    makeWidget        ({  5,  69}, {238, 69},         WidgetType::groupbox, WindowColour::secondary, STR_ADD_SET_MONEY     ), // add / set money group frame
    makeSpinnerWidgets({ 11,  92}, kCheatSpinnerSize, WidgetType::spinner,  WindowColour::secondary                        ), // money value
    makeWidget        ({ 11, 111}, kCheatButtonSize,  WidgetType::button,   WindowColour::secondary, STR_ADD_MONEY         ), // add money
    makeWidget        ({127, 111}, kCheatButtonSize,  WidgetType::button,   WindowColour::secondary, STR_SET_MONEY         ), // set money
    makeWidget        ({ 11, 145}, kCheatButtonSize,  WidgetType::button,   WindowColour::secondary, STR_CHEAT_CLEAR_LOAN  )  // Clear loan
);

static constexpr auto window_cheats_date_widgets = makeWidgets(
    kMainCheatWidgets,
    makeWidget        ({  5,  48}, {238, 99} ,        WidgetType::groupbox, WindowColour::secondary, STR_DATE_SET  ), // Date group
    makeSpinnerWidgets({120,  61}, kCheatSpinnerSize, WidgetType::spinner,  WindowColour::secondary                ), // Year box
    makeSpinnerWidgets({120,  82}, kCheatSpinnerSize, WidgetType::spinner,  WindowColour::secondary                ), // Month box
    makeSpinnerWidgets({120, 103}, kCheatSpinnerSize, WidgetType::spinner,  WindowColour::secondary                ), // Day box
    makeWidget        ({ 11, 122}, kCheatButtonSize,  WidgetType::button,   WindowColour::secondary, STR_DATE_SET  ), // Set Date
    makeWidget        ({127, 122}, kCheatButtonSize,  WidgetType::button,   WindowColour::secondary, STR_DATE_RESET)  // Reset Date
);

static constexpr auto window_cheats_guests_widgets = makeWidgets(
    kMainCheatWidgets,
    makeWidget({ 11,  48}, kCheatButtonSize,  WidgetType::button,   WindowColour::secondary, STR_CHEAT_LARGE_TRAM_GUESTS,     STR_CHEAT_LARGE_TRAM_GUESTS_TIP), // large tram
    makeWidget({127,  48}, kCheatButtonSize,  WidgetType::button,   WindowColour::secondary, STR_CHEAT_REMOVE_ALL_GUESTS,     STR_CHEAT_REMOVE_ALL_GUESTS_TIP), // remove all guests

    makeWidget({  5,  70}, {238, 206},        WidgetType::groupbox, WindowColour::secondary, STR_CHEAT_SET_GUESTS_PARAMETERS                                 ), // Guests parameters group frame
    makeWidget({183,  84}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MAX                                                         ), // happiness max
    makeWidget({127,  84}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MIN                                                         ), // happiness min
    makeWidget({183, 105}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MAX                                                         ), // energy max
    makeWidget({127, 105}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MIN                                                         ), // energy min
    makeWidget({183, 126}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MAX                                                         ), // hunger max
    makeWidget({127, 126}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MIN                                                         ), // hunger min
    makeWidget({183, 147}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MAX                                                         ), // thirst max
    makeWidget({127, 147}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MIN                                                         ), // thirst min
    makeWidget({183, 168}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MAX                                                         ), // nausea max
    makeWidget({127, 168}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MIN                                                         ), // nausea min
    makeWidget({183, 189}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MAX                                                         ), // nausea tolerance max
    makeWidget({127, 189}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MIN                                                         ), // nausea tolerance min
    makeWidget({183, 210}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MAX                                                         ), // toilet max
    makeWidget({127, 210}, kMinMaxButtonSize, WidgetType::button,   WindowColour::secondary, STR_MIN                                                         ), // toilet min
    makeWidget({127, 251}, kCheatButtonSize,  WidgetType::button,   WindowColour::secondary, STR_CHEAT_MORE_THAN_1                                           ), // ride intensity > 1
    makeWidget({ 11, 251}, kCheatButtonSize,  WidgetType::button,   WindowColour::secondary, STR_CHEAT_LESS_THAN_15                                          ), // ride intensity < 15

    makeWidget({  5, 258+15+6+2}, {238, 62},        WidgetType::groupbox, WindowColour::secondary, STR_CHEAT_GIVE_ALL_GUESTS                                       ), // Guests inventory group frame
    makeWidget({ 11, 279+15+6-3}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_CURRENCY_FORMAT                                             ), // give guests money
    makeWidget({127, 279+15+6-3}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_SHOP_ITEM_PLURAL_PARK_MAP                                   ), // give guests park maps
    makeWidget({ 11, 300+15+6-3}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_SHOP_ITEM_PLURAL_BALLOON                                    ), // give guests balloons
    makeWidget({127, 300+15+6-3}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_SHOP_ITEM_PLURAL_UMBRELLA                                   ), // give guests umbrellas

    makeWidget({  5, 342+6}, {238,  85},        WidgetType::groupbox, WindowColour::secondary, STR_GUEST_BEHAVIOUR                                             ), // Guests behaviour group frame
    makeWidget({ 11, 363+1}, kCheatCheckSize,   WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_IGNORE_INTENSITY,      STR_CHEAT_IGNORE_INTENSITY_TIP ), // guests ignore intensity
    makeWidget({ 11, 380+1}, kCheatCheckSize,   WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_IGNORE_PRICE,          STR_CHEAT_IGNORE_PRICE_TIP     ), // guests ignore price
    makeWidget({ 11, 397+1}, kCheatCheckSize,   WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_DISABLE_VANDALISM,     STR_CHEAT_DISABLE_VANDALISM_TIP), // disable vandalism
    makeWidget({ 11, 414+1}, kCheatCheckSize,   WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_DISABLE_LITTERING,     STR_CHEAT_DISABLE_LITTERING_TIP)  // disable littering
);

static constexpr auto window_cheats_staff_widgets = makeWidgets(
    kMainCheatWidgets,
    makeWidget        ({  5, 357-309}, {238,  35},       WidgetType::groupbox,     WindowColour::secondary, STR_CHEAT_STAFF_GROUP                                           ), // Staff group
    makeWidget        ({126, 371-309}, {111,  14},       WidgetType::dropdownMenu, WindowColour::secondary                                                                  ), // Staff speed
    makeWidget        ({225, 372-309}, { 11,  12},       WidgetType::button,       WindowColour::secondary, STR_DROPDOWN_GLYPH                                              ), // Staff speed

    makeWidget        ({  5, 257-168}, {238,  99},       WidgetType::groupbox,     WindowColour::secondary, STR_CHEAT_MAINTENANCE_GROUP                                     ), // Maintenance group
    makeWidget        ({ 11, 271-168}, kCheatButtonSize, WidgetType::button,       WindowColour::secondary, STR_CHEAT_REMOVE_LITTER                                         ), // Remove litter
    makeWidget        ({127, 271-168}, kCheatButtonSize, WidgetType::button,       WindowColour::secondary, STR_CHEAT_FIX_VANDALISM                                         ), // Fix vandalism
    makeWidget        ({ 11, 292-168}, kCheatButtonSize, WidgetType::button,       WindowColour::secondary, STR_CHEAT_CLEAR_GRASS                                           ), // Clear grass
    makeWidget        ({127, 292-168}, kCheatButtonSize, WidgetType::button,       WindowColour::secondary, STR_CHEAT_MOWED_GRASS                                           ), // Mowed grass
    makeWidget        ({ 11, 313-168}, kCheatButtonSize, WidgetType::button,       WindowColour::secondary, STR_CHEAT_WATER_PLANTS                                          ), // Water plants
    makeWidget        ({ 11, 334-164}, kCheatCheckSize,  WidgetType::checkbox,     WindowColour::secondary, STR_CHEAT_DISABLE_PLANT_AGING, STR_CHEAT_DISABLE_PLANT_AGING_TIP)  // Disable plant ageing
);

static constexpr auto window_cheats_park_widgets = makeWidgets(
    kMainCheatWidgets,
    makeWidget        ({  5,  48}, {238,  60},       WidgetType::groupbox, WindowColour::secondary, STR_CHEAT_GENERAL_GROUP                                                           ), // General group
    makeWidget        ({ 11,  62}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_CHEAT_OWN_ALL_LAND,                 STR_CHEAT_OWN_ALL_LAND_TIP                ), // Own all land
    makeWidget        ({127,  62}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_CHEAT_REMOVE_PARK_FENCES,           STR_CHEAT_REMOVE_PARK_FENCES_TIP          ), // Remove park fences
    makeWidget        ({ 11,  83}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_CHEAT_OPEN_PARK,                    STR_CHEAT_OPEN_PARK_TIP                   ), // open / close park

    makeWidget        ({  5, 113}, {238,  75},       WidgetType::groupbox, WindowColour::secondary, STR_CHEAT_OBJECTIVE_GROUP                                                         ), // Objective group
    makeWidget        ({ 11, 128}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_NEVERENDING_MARKETING,        STR_CHEAT_NEVERENDING_MARKETING_TIP       ), // never ending marketing campaigns
    makeWidget        ({ 11, 145}, {281,  12},       WidgetType::checkbox, WindowColour::secondary, STR_FORCE_PARK_RATING                                                             ), // Force park rating
    makeSpinnerWidgets({156, 143}, { 81,  14},       WidgetType::spinner,  WindowColour::secondary                                                                                    ), // park rating (3 widgets)
    makeWidget        ({ 11, 163}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_CHEAT_WIN_SCENARIO                                                            ), // Win scenario
    makeWidget        ({127, 163}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_CHEAT_HAVE_FUN                                                                ), // Have fun!

    makeWidget        ({  5, 192}, {238,  68},       WidgetType::groupbox, WindowColour::secondary, STR_CHEAT_GROUP_CONSTRUCTION                                                      ), // Construction group
    makeWidget        ({ 11, 207}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_BUILD_IN_PAUSE_MODE,          STR_CHEAT_BUILD_IN_PAUSE_MODE_TIP         ), // Build in pause mode
    makeWidget        ({ 11, 224}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_ALLOW_PATH_AS_QUEUE,          STR_CHEAT_ALLOW_PATH_AS_QUEUE_TIP         ), // Allow regular footpaths as queue path
    makeWidget        ({ 11, 241}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_ALLOW_SPECIAL_COLOUR_SCHEMES, STR_CHEAT_ALLOW_SPECIAL_COLOUR_SCHEMES_TIP)  // Allow special colours in dropdown
);

static constexpr auto window_cheats_rides_widgets = makeWidgets(
    kMainCheatWidgets,
    makeWidget({ 11,  48}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_CHEAT_FIX_ALL_RIDES,                        STR_CHEAT_FIX_ALL_RIDES_TIP                    ), // Fix all rides
    makeWidget({127,  48}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_CHEAT_RENEW_RIDES,                          STR_CHEAT_RENEW_RIDES_TIP                      ), // Renew rides
    makeWidget({127,  69}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_CHEAT_RESET_CRASH_STATUS,                   STR_CHEAT_RESET_CRASH_STATUS_TIP               ), // Reset crash status
    makeWidget({ 11,  69}, kCheatButtonSize, WidgetType::button,   WindowColour::secondary, STR_CHEAT_10_MINUTE_INSPECTIONS,                STR_CHEAT_10_MINUTE_INSPECTIONS_TIP            ), // 10 minute inspections

    makeWidget({  5, 95},  {238, 87},        WidgetType::groupbox, WindowColour::secondary, STR_CHEAT_GROUP_CONSTRUCTION                                                                   ), // Construction group
    makeWidget({ 11, 111}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_ENABLE_ALL_DRAWABLE_TRACK_PIECES,     STR_CHEAT_ENABLE_ALL_DRAWABLE_TRACK_PIECES_TIP ), // Show all drawable track pieces
    makeWidget({ 11, 128}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_ENABLE_CHAIN_LIFT_ON_ALL_TRACK,       STR_CHEAT_ENABLE_CHAIN_LIFT_ON_ALL_TRACK_TIP   ), // Enable chain lift on all track
    makeWidget({ 11, 145}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_ALLOW_TRACK_PLACE_INVALID_HEIGHTS,    STR_CHEAT_ALLOW_TRACK_PLACE_INVALID_HEIGHTS_TIP), // Allow track place at invalid heights
    makeWidget({ 11, 162}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_MAKE_DESTRUCTABLE,                    STR_CHEAT_MAKE_DESTRUCTABLE_TIP                ), // All destructible

    makeWidget({  5, 186}, {238, 102},       WidgetType::groupbox, WindowColour::secondary, STR_CHEAT_GROUP_OPERATION                                                                      ), // Operation group
    makeWidget({ 11, 201}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_SHOW_ALL_OPERATING_MODES                                                             ), // Show all operating modes
    makeWidget({ 11, 218}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_UNLOCK_OPERATING_LIMITS,              STR_CHEAT_UNLOCK_OPERATING_LIMITS_TIP          ), // 410 km/h lift hill etc.
    makeWidget({ 11, 235}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_DISABLE_BRAKES_FAILURE,               STR_CHEAT_DISABLE_BRAKES_FAILURE_TIP           ), // Disable brakes failure
    makeWidget({ 11, 252}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_DISABLE_BREAKDOWNS,                   STR_CHEAT_DISABLE_BREAKDOWNS_TIP               ), // Disable all breakdowns
    makeWidget({ 11, 269}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_DISABLE_RIDE_VALUE_AGING,             STR_CHEAT_DISABLE_RIDE_VALUE_AGING_TIP         ), // Disable ride ageing

    makeWidget({  5, 292}, {238, 120},        WidgetType::groupbox, WindowColour::secondary, STR_CHEAT_GROUP_AVAILABILITY                                                                   ), // Availability group
    makeWidget({ 11, 308}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_ALLOW_ARBITRARY_RIDE_TYPE_CHANGES,    STR_CHEAT_ALLOW_ARBITRARY_RIDE_TYPE_CHANGES_TIP), // Allow arbitrary ride type changes
    makeWidget({ 11, 325}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_SHOW_VEHICLES_FROM_OTHER_TRACK_TYPES                                                 ), // Show vehicles from other track types
    makeWidget({ 11, 342}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_DISABLE_TRAIN_LENGTH_LIMIT,           STR_CHEAT_DISABLE_TRAIN_LENGTH_LIMIT_TIP       ), // Disable train length limits
    makeWidget({ 11, 359}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_CHEAT_IGNORE_RESEARCH_STATUS,               STR_CHEAT_IGNORE_RESEARCH_STATUS_TIP           ), // Ignore Research Status
    makeWidget({ 11, 376}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_ALLOW_INCOMPLETE_RIDES                    ), // Allow incomplete rides
    makeWidget({ 11, 393}, kCheatCheckSize,  WidgetType::checkbox, WindowColour::secondary, STR_NORMALIZE_RIDE_CRASHES                     ) // Normalize ride crashes
);

static constexpr auto window_cheats_weather_widgets = makeWidgets(
    kMainCheatWidgets,
    makeWidget        ({  5,  48}, {238,  50},       WidgetType::groupbox,     WindowColour::secondary, STR_CHEAT_WEATHER_GROUP                                      ), // Weather group
    makeWidget        ({126,  62}, {111,  14},       WidgetType::dropdownMenu, WindowColour::secondary, kStringIdNone,                   STR_CHANGE_WEATHER_TOOLTIP  ), // Force weather
    makeWidget        ({225,  63}, { 11,  12},       WidgetType::button,       WindowColour::secondary, STR_DROPDOWN_GLYPH,              STR_CHANGE_WEATHER_TOOLTIP  ), // Force weather
    makeWidget        ({ 11,  80}, kCheatCheckSize,  WidgetType::checkbox,     WindowColour::secondary, STR_CHEAT_FREEZE_WEATHER,        STR_CHEAT_FREEZE_WEATHER_TIP), // Freeze weather
    makeWidget        ({  5, 102}, {238,  37},       WidgetType::groupbox,     WindowColour::secondary, STR_FAUNA                                                    ), // Fauna group
    makeWidget        ({ 11, 115}, kCheatButtonSize, WidgetType::button,       WindowColour::secondary, STR_CREATE_DUCKS,                STR_CREATE_DUCKS_TIP        ), // Create ducks
    makeWidget        ({127, 115}, kCheatButtonSize, WidgetType::button,       WindowColour::secondary, STR_REMOVE_DUCKS,                STR_REMOVE_DUCKS_TIP        )  // Remove ducks
);

static constexpr std::span<const Widget> window_cheats_page_widgets[] =
{
    window_cheats_money_widgets,
    window_cheats_date_widgets,
    window_cheats_guests_widgets,
    window_cheats_staff_widgets,
    window_cheats_park_widgets,
    window_cheats_rides_widgets,
    window_cheats_weather_widgets,
};

static uint64_t window_cheats_page_hold_down_widgets[] = {
    (1uLL << WIDX_MONEY_SPINNER_INCREMENT) |
    (1uLL << WIDX_MONEY_SPINNER_DECREMENT) |
    (1uLL << WIDX_ADD_MONEY),

    (1uLL << WIDX_YEAR_UP) |
    (1uLL << WIDX_YEAR_DOWN) |
    (1uLL << WIDX_MONTH_UP) |
    (1uLL << WIDX_MONTH_DOWN) |
    (1uLL << WIDX_DAY_UP) |
    (1uLL << WIDX_DAY_DOWN),

    0,

    0,

    (1uLL << WIDX_INCREASE_PARK_RATING) |
    (1uLL << WIDX_DECREASE_PARK_RATING),

    0,

    0,
};

static StringId window_cheats_page_titles[] = {
    STR_CHEAT_TITLE_FINANCIAL,
    STR_CHEAT_TITLE_DATE,
    STR_CHEAT_TITLE_GUEST,
    STR_CHEAT_TITLE_STAFF,
    STR_CHEAT_TITLE_PARK,
    STR_CHEAT_TITLE_RIDE,
    STR_CHEAT_TITLE_WEATHER,
};
    // clang-format on

    class CheatsWindow final : public Window
    {
    private:
        char _moneySpinnerText[kMoneyStringMaxlength]{};
        money64 _moneySpinnerValue = kCheatsMoneyDefault;
        int32_t _parkRatingSpinnerValue{};
        int32_t _yearSpinnerValue = 1;
        int32_t _monthSpinnerValue = 1;
        int32_t _daySpinnerValue = 1;

    public:
        void OnOpen() override
        {
            SetPage(WINDOW_CHEATS_PAGE_MONEY);
            _parkRatingSpinnerValue = Park::GetForcedRating() >= 0 ? Park::GetForcedRating() : 999;
        }

        void OnUpdate() override
        {
            frame_no++;
            InvalidateWidget(WIDX_TAB_1 + page);
        }

        void OnMouseDown(WidgetIndex widgetIndex) override
        {
            switch (page)
            {
                case WINDOW_CHEATS_PAGE_MONEY:
                    OnMouseDownMoney(widgetIndex);
                    break;
                case WINDOW_CHEATS_PAGE_DATE:
                    OnMouseDownDate(widgetIndex);
                    break;
                case WINDOW_CHEATS_PAGE_PARK:
                    OnMouseDownPark(widgetIndex);
                    break;
                case WINDOW_CHEATS_PAGE_STAFF:
                    OnMouseDownStaff(widgetIndex);
                    break;
                case WINDOW_CHEATS_PAGE_WEATHER:
                    OnMouseDownWeather(widgetIndex);
                    break;
            }
        }

        void OnMouseUp(WidgetIndex widgetIndex) override
        {
            switch (widgetIndex)
            {
                case WIDX_CLOSE:
                    Close();
                    break;
                case WIDX_TAB_1:
                case WIDX_TAB_2:
                case WIDX_TAB_3:
                case WIDX_TAB_4:
                case WIDX_TAB_5:
                case WIDX_TAB_6:
                case WIDX_TAB_7:
                    SetPage(widgetIndex - WIDX_TAB_1);
                    break;
                default:
                    switch (page)
                    {
                        case WINDOW_CHEATS_PAGE_MONEY:
                            OnMouseUpMoney(widgetIndex);
                            break;
                        case WINDOW_CHEATS_PAGE_GUESTS:
                            OnMouseUpGuests(widgetIndex);
                            break;
                        case WINDOW_CHEATS_PAGE_PARK:
                            OnMouseUpPark(widgetIndex);
                            break;
                        case WINDOW_CHEATS_PAGE_RIDES:
                            OnMouseUpRides(widgetIndex);
                            break;
                        case WINDOW_CHEATS_PAGE_STAFF:
                            OnMouseUpStaff(widgetIndex);
                            break;
                        case WINDOW_CHEATS_PAGE_WEATHER:
                            OnMouseUpWeather(widgetIndex);
                            break;
                    }
                    break;
            }
        }

        void OnDropdown(WidgetIndex widgetIndex, int32_t selectedIndex) override
        {
            if (page == WINDOW_CHEATS_PAGE_STAFF)
            {
                OnDropdownStaff(widgetIndex, selectedIndex);
            }
            else if (page == WINDOW_CHEATS_PAGE_WEATHER)
            {
                OnDropdownWeather(widgetIndex, selectedIndex);
            }
        }

        void OnPrepareDraw() override
        {
            pressed_widgets = 0;
            disabled_widgets = 0;

            // Set correct active tab
            for (auto i = 0; i < WINDOW_CHEATS_PAGE_COUNT; i++)
                SetWidgetPressed(WIDX_TAB_1 + i, false);
            SetWidgetPressed(WIDX_TAB_1 + page, true);

            // Set title
            widgets[WIDX_TITLE].text = window_cheats_page_titles[page];

            auto& gameState = getGameState();
            switch (page)
            {
                case WINDOW_CHEATS_PAGE_MONEY:
                {
                    if (isInEditorMode())
                    {
                        SetWidgetDisabled(WIDX_NO_MONEY, true);
                    }

                    auto moneyDisabled = (gameState.park.Flags & PARK_FLAGS_NO_MONEY) != 0;
                    SetCheckboxValue(WIDX_NO_MONEY, moneyDisabled);
                    SetWidgetDisabled(WIDX_ADD_SET_MONEY_GROUP, moneyDisabled);
                    SetWidgetDisabled(WIDX_MONEY_SPINNER, moneyDisabled);
                    SetWidgetDisabled(WIDX_MONEY_SPINNER_INCREMENT, moneyDisabled);
                    SetWidgetDisabled(WIDX_MONEY_SPINNER_DECREMENT, moneyDisabled);
                    SetWidgetDisabled(WIDX_ADD_MONEY, moneyDisabled);
                    SetWidgetDisabled(WIDX_SET_MONEY, moneyDisabled);
                    SetWidgetDisabled(WIDX_CLEAR_LOAN, moneyDisabled);
                    break;
                }
                case WINDOW_CHEATS_PAGE_GUESTS:
                {
                    auto ft = Formatter::Common();
                    ft.Add<money64>(1000.00_GBP);
                    SetCheckboxValue(WIDX_GUEST_IGNORE_RIDE_INTENSITY, gameState.cheats.ignoreRideIntensity);
                    SetCheckboxValue(WIDX_GUEST_IGNORE_PRICE, gameState.cheats.ignorePrice);
                    SetCheckboxValue(WIDX_DISABLE_VANDALISM, gameState.cheats.disableVandalism);
                    SetCheckboxValue(WIDX_DISABLE_LITTERING, gameState.cheats.disableLittering);
                    break;
                }
                case WINDOW_CHEATS_PAGE_PARK:
                    widgets[WIDX_OPEN_CLOSE_PARK].text = STR_CHEAT_OPEN_PARK;
                    if (gameState.park.Flags & PARK_FLAGS_PARK_OPEN)
                        widgets[WIDX_OPEN_CLOSE_PARK].text = STR_CHEAT_CLOSE_PARK;

                    SetCheckboxValue(WIDX_FORCE_PARK_RATING, Park::GetForcedRating() >= 0);
                    SetCheckboxValue(WIDX_NEVERENDING_MARKETING, gameState.cheats.neverendingMarketing);
                    SetCheckboxValue(WIDX_ALLOW_BUILD_IN_PAUSE_MODE, gameState.cheats.buildInPauseMode);
                    SetCheckboxValue(WIDX_ALLOW_REGULAR_PATH_AS_QUEUE, gameState.cheats.allowRegularPathAsQueue);
                    SetCheckboxValue(WIDX_ALLOW_SPECIAL_COLOUR_SCHEMES, gameState.cheats.allowSpecialColourSchemes);
                    break;
                case WINDOW_CHEATS_PAGE_RIDES:
                    SetCheckboxValue(WIDX_UNLOCK_OPERATING_LIMITS, gameState.cheats.unlockOperatingLimits);
                    SetCheckboxValue(WIDX_DISABLE_BRAKES_FAILURE, gameState.cheats.disableBrakesFailure);
                    SetCheckboxValue(WIDX_DISABLE_ALL_BREAKDOWNS, gameState.cheats.disableAllBreakdowns);
                    SetCheckboxValue(WIDX_SHOW_ALL_OPERATING_MODES, gameState.cheats.showAllOperatingModes);
                    SetCheckboxValue(
                        WIDX_SHOW_VEHICLES_FROM_OTHER_TRACK_TYPES, gameState.cheats.showVehiclesFromOtherTrackTypes);
                    SetCheckboxValue(WIDX_DISABLE_TRAIN_LENGTH_LIMITS, gameState.cheats.disableTrainLengthLimit);
                    SetCheckboxValue(WIDX_ENABLE_CHAIN_LIFT_ON_ALL_TRACK, gameState.cheats.enableChainLiftOnAllTrack);
                    SetCheckboxValue(WIDX_ENABLE_ARBITRARY_RIDE_TYPE_CHANGES, gameState.cheats.allowArbitraryRideTypeChanges);
                    SetCheckboxValue(WIDX_DISABLE_RIDE_VALUE_AGING, gameState.cheats.disableRideValueAging);
                    SetCheckboxValue(WIDX_IGNORE_RESEARCH_STATUS, gameState.cheats.ignoreResearchStatus);
                    SetCheckboxValue(WIDX_ENABLE_ALL_DRAWABLE_TRACK_PIECES, gameState.cheats.enableAllDrawableTrackPieces);
                    SetCheckboxValue(WIDX_ALLOW_TRACK_PLACE_INVALID_HEIGHTS, gameState.cheats.allowTrackPlaceInvalidHeights);
                    SetCheckboxValue(WIDX_MAKE_DESTRUCTIBLE, gameState.cheats.makeAllDestructible);
                    SetCheckboxValue(WIDX_ALLOW_INCOMPLETE_RIDES, gameState.cheats.allowIncompleteRides);
                    SetCheckboxValue(WIDX_NORMALIZE_RIDE_CRASHES, gameState.cheats.normalizeRideCrashes);
                    break;
                case WINDOW_CHEATS_PAGE_STAFF:
                    SetCheckboxValue(WIDX_DISABLE_PLANT_AGING, gameState.cheats.disablePlantAging);
                    break;
                case WINDOW_CHEATS_PAGE_WEATHER:
                    SetCheckboxValue(WIDX_FREEZE_WEATHER, gameState.cheats.freezeWeather);
                    break;
            }

            // Current weather
            if (page == WINDOW_CHEATS_PAGE_WEATHER)
            {
                widgets[WIDX_WEATHER].text = WeatherTypes[EnumValue(gameState.weatherCurrent.weatherType)];
            }

            // Staff speed
            if (page == WINDOW_CHEATS_PAGE_STAFF)
            {
                widgets[WIDX_STAFF_SPEED].text = _staffSpeedNames[EnumValue(gameState.cheats.selectedStaffSpeed)];
            }

            if (isInEditorMode())
            {
                SetWidgetDisabled(WIDX_TAB_2, true);
                SetWidgetDisabled(WIDX_TAB_3, true);
                UpdateTabPositions();
            }
        }

        void OnDraw(RenderTarget& rt) override
        {
            DrawWidgets(rt);
            DrawTabImages(rt);

            static constexpr int16_t _xLcol = 14;
            static constexpr int16_t _xRcol = 208;

            if (page == WINDOW_CHEATS_PAGE_MONEY)
            {
                auto colour = colours[1];
                auto ft = Formatter();
                ft.Add<money64>(_moneySpinnerValue);
                if (IsWidgetDisabled(WIDX_MONEY_SPINNER))
                {
                    colour.setFlag(ColourFlag::inset, true);
                }

                auto& widget = widgets[WIDX_MONEY_SPINNER];
                DrawTextBasic(
                    rt, windowPos + ScreenCoordsXY{ _xLcol, widget.top + 2 }, STR_BOTTOM_TOOLBAR_CASH, ft, { colour });
            }
            else if (page == WINDOW_CHEATS_PAGE_DATE)
            {
                auto& yearBox = widgets[WIDX_YEAR_BOX];
                DrawTextBasic(rt, windowPos + ScreenCoordsXY{ _xLcol, yearBox.top + 2 }, STR_YEAR);

                auto& monthBox = widgets[WIDX_MONTH_BOX];
                DrawTextBasic(rt, windowPos + ScreenCoordsXY{ _xLcol, monthBox.top + 2 }, STR_MONTH);

                auto& dayBox = widgets[WIDX_DAY_BOX];
                DrawTextBasic(rt, windowPos + ScreenCoordsXY{ _xLcol, dayBox.top + 2 }, STR_DAY);

                auto ft = Formatter();
                ft.Add<int32_t>(_yearSpinnerValue);
                DrawTextBasic(
                    rt, windowPos + ScreenCoordsXY{ _xRcol, yearBox.top + 2 }, STR_FORMAT_INTEGER, ft,
                    { colours[1], TextAlignment::RIGHT });

                ft = Formatter();
                int32_t actual_month = _monthSpinnerValue - 1;
                ft.Add<int32_t>(actual_month);
                DrawTextBasic(
                    rt, windowPos + ScreenCoordsXY{ _xRcol, monthBox.top + 2 }, STR_FORMAT_MONTH, ft,
                    { colours[1], TextAlignment::RIGHT });

                ft = Formatter();
                ft.Add<int32_t>(_daySpinnerValue);
                DrawTextBasic(
                    rt, windowPos + ScreenCoordsXY{ _xRcol, dayBox.top + 2 }, STR_FORMAT_INTEGER, ft,
                    { colours[1], TextAlignment::RIGHT });
            }
            else if (page == WINDOW_CHEATS_PAGE_PARK)
            {
                {
                    auto ft = Formatter();
                    ft.Add<int32_t>(_parkRatingSpinnerValue);

                    auto& widget = widgets[WIDX_PARK_RATING_SPINNER];
                    DrawTextBasic(
                        rt, windowPos + ScreenCoordsXY{ widget.left + 1, widget.top + 2 }, STR_FORMAT_INTEGER, ft,
                        { colours[1] });
                }
            }
            else if (page == WINDOW_CHEATS_PAGE_STAFF)
            {
                {
                    auto& widget = widgets[WIDX_STAFF_SPEED];
                    DrawTextBasic(rt, windowPos + ScreenCoordsXY{ _xLcol - 3, widget.top + 1 }, STR_CHEAT_STAFF_SPEED);
                }
            }
            else if (page == WINDOW_CHEATS_PAGE_GUESTS)
            {
                {
                    auto& widget = widgets[WIDX_GUEST_HAPPINESS_MIN];
                    DrawTextBasic(rt, windowPos + ScreenCoordsXY{ _xLcol - 3, widget.top + 2 }, STR_CHEAT_GUEST_HAPPINESS);
                }
                {
                    auto& widget = widgets[WIDX_GUEST_ENERGY_MIN];
                    DrawTextBasic(rt, windowPos + ScreenCoordsXY{ _xLcol - 3, widget.top + 2 }, STR_CHEAT_GUEST_ENERGY);
                }
                {
                    auto& widget = widgets[WIDX_GUEST_HUNGER_MIN];
                    DrawTextBasic(rt, windowPos + ScreenCoordsXY{ _xLcol - 3, widget.top + 2 }, STR_CHEAT_GUEST_HUNGER);
                }
                {
                    auto& widget = widgets[WIDX_GUEST_THIRST_MIN];
                    DrawTextBasic(rt, windowPos + ScreenCoordsXY{ _xLcol - 3, widget.top + 2 }, STR_CHEAT_GUEST_THIRST);
                }
                {
                    auto& widget = widgets[WIDX_GUEST_NAUSEA_MIN];
                    DrawTextBasic(rt, windowPos + ScreenCoordsXY{ _xLcol - 3, widget.top + 2 }, STR_CHEAT_GUEST_NAUSEA);
                }
                {
                    auto& widget = widgets[WIDX_GUEST_NAUSEA_TOLERANCE_MIN];
                    DrawTextBasic(
                        rt, windowPos + ScreenCoordsXY{ _xLcol - 3, widget.top + 2 }, STR_CHEAT_GUEST_NAUSEA_TOLERANCE);
                }
                {
                    auto& widget = widgets[WIDX_GUEST_TOILET_MIN];
                    DrawTextBasic(rt, windowPos + ScreenCoordsXY{ _xLcol - 3, widget.top + 2 }, STR_CHEAT_GUEST_TOILET);
                }
                {
                    auto& widget = widgets[WIDX_GUEST_RIDE_INTENSITY_LESS_THAN_15];
                    DrawTextBasic(
                        rt, windowPos + ScreenCoordsXY{ _xLcol - 3, widget.top - 17 }, STR_CHEAT_GUEST_PREFERRED_INTENSITY);
                }
            }
            else if (page == WINDOW_CHEATS_PAGE_WEATHER)
            {
                {
                    auto& widget = widgets[WIDX_WEATHER];
                    DrawTextBasic(rt, windowPos + ScreenCoordsXY{ _xLcol - 3, widget.top + 1 }, STR_CHANGE_WEATHER);
                }
            }
        }

        void OnTextInput(WidgetIndex widgetIndex, std::string_view text) override
        {
            if (page == WINDOW_CHEATS_PAGE_MONEY && widgetIndex == WIDX_MONEY_SPINNER)
            {
                auto val = StringToMoney(std::string(text).c_str());
                if (val != kMoney64Undefined)
                {
                    _moneySpinnerValue = val;
                }
                Invalidate();
            }
        }

        OpenRCT2String OnTooltip(WidgetIndex widgetIndex, StringId fallback) override
        {
            if (page == WINDOW_CHEATS_PAGE_RIDES && widgetIndex == WIDX_UNLOCK_OPERATING_LIMITS)
            {
                auto ft = Formatter{};
                ft.Add<uint16_t>(255);
                return { fallback, ft };
            }
            return { fallback, {} };
        }

    private:
        void SetPage(int32_t p)
        {
            // Skip setting page if we're already on this page, unless we're initialising the window
            if (page == p && !widgets.empty())
                return;

            page = p;
            frame_no = 0;

            hold_down_widgets = window_cheats_page_hold_down_widgets[p];
            pressed_widgets = 0;
            SetWidgets(window_cheats_page_widgets[p]);

            auto maxY = 0;
            for (WidgetIndex widgetIdx = WIDX_TAB_CONTENT; widgetIdx < widgets.size(); widgetIdx++)
            {
                auto& widget = widgets[widgetIdx];
                maxY = std::max<int32_t>(maxY, widget.bottom);
            }
            maxY += 6;

            if (maxY != height)
            {
                Invalidate();
                height = maxY;
                ResizeFrame();
                Invalidate();
            }
        }

        void UpdateTabPositions()
        {
            constexpr WidgetIndex tabs[] = {
                WIDX_TAB_1, WIDX_TAB_2, WIDX_TAB_3, WIDX_TAB_4, WIDX_TAB_5, WIDX_TAB_6, WIDX_TAB_7,
            };

            auto left = kTabStart;
            for (auto tab : tabs)
            {
                widgets[tab].left = left;
                if (!IsWidgetDisabled(tab))
                {
                    left += kTabWidth;
                }
            }
        }

        void DrawTabImages(RenderTarget& rt)
        {
            // Money tab
            if (!IsWidgetDisabled(WIDX_TAB_1))
            {
                uint32_t sprite_idx = SPR_TAB_FINANCES_SUMMARY_0;
                if (page == WINDOW_CHEATS_PAGE_MONEY)
                    sprite_idx += (frame_no / 2) % 8;
                GfxDrawSprite(
                    rt, ImageId(sprite_idx), windowPos + ScreenCoordsXY{ widgets[WIDX_TAB_1].left, widgets[WIDX_TAB_1].top });
            }

            // Date tab
            if (!IsWidgetDisabled(WIDX_TAB_2))
            {
                uint32_t sprite_idx = SPR_TAB_TIMER_0;
                if (page == WINDOW_CHEATS_PAGE_DATE)
                    sprite_idx += (frame_no / 8) % 8;
                GfxDrawSprite(
                    rt, ImageId(sprite_idx), windowPos + ScreenCoordsXY{ widgets[WIDX_TAB_2].left, widgets[WIDX_TAB_2].top });
            }

            // Guests tab
            if (!IsWidgetDisabled(WIDX_TAB_3))
            {
                uint32_t sprite_idx = SPR_TAB_GUESTS_0;
                if (page == WINDOW_CHEATS_PAGE_GUESTS)
                    sprite_idx += (frame_no / 3) % 8;
                GfxDrawSprite(
                    rt, ImageId(sprite_idx), windowPos + ScreenCoordsXY{ widgets[WIDX_TAB_3].left, widgets[WIDX_TAB_3].top });
            }

            // Staff tab
            if (!IsWidgetDisabled(WIDX_TAB_4))
            {
                uint32_t sprite_idx = SPR_MECHANIC;
                GfxDrawSprite(
                    rt, ImageId(sprite_idx),
                    windowPos + ScreenCoordsXY{ widgets[WIDX_TAB_4].left + 2, widgets[WIDX_TAB_4].top + 1 });
            }

            // Park tab
            if (!IsWidgetDisabled(WIDX_TAB_5))
            {
                GfxDrawSprite(
                    rt, ImageId(SPR_TAB_PARK), windowPos + ScreenCoordsXY{ widgets[WIDX_TAB_5].left, widgets[WIDX_TAB_5].top });
            }

            // Rides tab
            if (!IsWidgetDisabled(WIDX_TAB_6))
            {
                uint32_t sprite_idx = SPR_TAB_RIDE_0;
                if (page == WINDOW_CHEATS_PAGE_RIDES)
                    sprite_idx += (frame_no / 4) % 16;
                GfxDrawSprite(
                    rt, ImageId(sprite_idx), windowPos + ScreenCoordsXY{ widgets[WIDX_TAB_6].left, widgets[WIDX_TAB_6].top });
            }

            // Nature/weather tab
            if (!IsWidgetDisabled(WIDX_TAB_7))
            {
                uint32_t sprite_idx = SPR_WEATHER_SUN_CLOUD;
                GfxDrawSprite(
                    rt, ImageId(sprite_idx),
                    windowPos + ScreenCoordsXY{ widgets[WIDX_TAB_7].left + 2, widgets[WIDX_TAB_7].top + 4 });
            }
        }

        void OnMouseDownMoney(WidgetIndex widgetIndex)
        {
            switch (widgetIndex)
            {
                case WIDX_MONEY_SPINNER_INCREMENT:
                    _moneySpinnerValue = AddClamp<money64>(
                        kCheatsMoneyIncrement * (_moneySpinnerValue / kCheatsMoneyIncrement), kCheatsMoneyIncrement);
                    InvalidateWidget(WIDX_MONEY_SPINNER);
                    break;
                case WIDX_MONEY_SPINNER_DECREMENT:
                    _moneySpinnerValue = AddClamp<money64>(
                        kCheatsMoneyIncrement * (_moneySpinnerValue / kCheatsMoneyIncrement), -kCheatsMoneyIncrement);
                    InvalidateWidget(WIDX_MONEY_SPINNER);
                    break;
                case WIDX_ADD_MONEY:
                    CheatsSet(CheatType::AddMoney, _moneySpinnerValue);
                    break;
            }
        }

        void OnMouseDownDate(WidgetIndex widgetIndex)
        {
            auto* windowMgr = Ui::GetWindowManager();

            switch (widgetIndex)
            {
                case WIDX_YEAR_UP:
                    _yearSpinnerValue++;
                    _yearSpinnerValue = std::clamp(_yearSpinnerValue, 1, kMaxYear);
                    InvalidateWidget(WIDX_YEAR_BOX);
                    break;
                case WIDX_YEAR_DOWN:
                    _yearSpinnerValue--;
                    _yearSpinnerValue = std::clamp(_yearSpinnerValue, 1, kMaxYear);
                    InvalidateWidget(WIDX_YEAR_BOX);
                    break;
                case WIDX_MONTH_UP:
                    _monthSpinnerValue++;
                    _monthSpinnerValue = std::clamp(_monthSpinnerValue, 1, static_cast<int32_t>(MONTH_COUNT));
                    _daySpinnerValue = std::clamp(
                        _daySpinnerValue, 1, static_cast<int32_t>(Date::GetDaysInMonth(_monthSpinnerValue - 1)));
                    InvalidateWidget(WIDX_MONTH_BOX);
                    InvalidateWidget(WIDX_DAY_BOX);
                    break;
                case WIDX_MONTH_DOWN:
                    _monthSpinnerValue--;
                    _monthSpinnerValue = std::clamp(_monthSpinnerValue, 1, static_cast<int32_t>(MONTH_COUNT));
                    _daySpinnerValue = std::clamp(
                        _daySpinnerValue, 1, static_cast<int32_t>(Date::GetDaysInMonth(_monthSpinnerValue - 1)));
                    InvalidateWidget(WIDX_MONTH_BOX);
                    InvalidateWidget(WIDX_DAY_BOX);
                    break;
                case WIDX_DAY_UP:
                    _daySpinnerValue++;
                    _daySpinnerValue = std::clamp(
                        _daySpinnerValue, 1, static_cast<int32_t>(Date::GetDaysInMonth(_monthSpinnerValue - 1)));
                    InvalidateWidget(WIDX_DAY_BOX);
                    break;
                case WIDX_DAY_DOWN:
                    _daySpinnerValue--;
                    _daySpinnerValue = std::clamp(
                        _daySpinnerValue, 1, static_cast<int32_t>(Date::GetDaysInMonth(_monthSpinnerValue - 1)));
                    InvalidateWidget(WIDX_DAY_BOX);
                    break;
                case WIDX_DATE_SET:
                {
                    auto setDateAction = ParkSetDateAction(_yearSpinnerValue - 1, _monthSpinnerValue - 1, _daySpinnerValue - 1);
                    GameActions::Execute(&setDateAction);
                    windowMgr->InvalidateByClass(WindowClass::BottomToolbar);
                    break;
                }
                case WIDX_DATE_RESET:
                {
                    auto setDateAction = ParkSetDateAction(0, 0, 0);
                    GameActions::Execute(&setDateAction);
                    windowMgr->InvalidateByClass(WindowClass::BottomToolbar);
                    InvalidateWidget(WIDX_YEAR_BOX);
                    InvalidateWidget(WIDX_MONTH_BOX);
                    InvalidateWidget(WIDX_DAY_BOX);
                    break;
                }
            }
        }

        void OnMouseUpMoney(WidgetIndex widgetIndex)
        {
            switch (widgetIndex)
            {
                case WIDX_NO_MONEY:
                    CheatsSet(CheatType::NoMoney, getGameState().park.Flags & PARK_FLAGS_NO_MONEY ? 0 : 1);
                    break;
                case WIDX_MONEY_SPINNER:
                    MoneyToString(_moneySpinnerValue, _moneySpinnerText, kMoneyStringMaxlength, false);
                    WindowTextInputRawOpen(
                        this, WIDX_MONEY_SPINNER, STR_ENTER_NEW_VALUE, STR_ENTER_NEW_VALUE, {}, _moneySpinnerText,
                        kMoneyStringMaxlength);
                    break;
                case WIDX_SET_MONEY:
                    CheatsSet(CheatType::SetMoney, _moneySpinnerValue);
                    break;
                case WIDX_CLEAR_LOAN:
                    CheatsSet(CheatType::ClearLoan);
                    break;
            }
        }

        void OnMouseDownPark(WidgetIndex widgetIndex)
        {
            switch (widgetIndex)
            {
                case WIDX_INCREASE_PARK_RATING:
                    _parkRatingSpinnerValue = std::min(999, 10 * (_parkRatingSpinnerValue / 10 + 1));
                    InvalidateWidget(WIDX_PARK_RATING_SPINNER);
                    if (Park::GetForcedRating() >= 0)
                    {
                        auto cheatSetAction = CheatSetAction(CheatType::SetForcedParkRating, _parkRatingSpinnerValue);
                        GameActions::Execute(&cheatSetAction);
                    }
                    break;
                case WIDX_DECREASE_PARK_RATING:
                    _parkRatingSpinnerValue = std::max(0, 10 * (_parkRatingSpinnerValue / 10 - 1));
                    InvalidateWidget(WIDX_PARK_RATING_SPINNER);
                    if (Park::GetForcedRating() >= 0)
                    {
                        CheatsSet(CheatType::SetForcedParkRating, _parkRatingSpinnerValue);
                    }
                    break;
            }
        }

        void OnMouseDownStaff(WidgetIndex widgetIndex)
        {
            auto* widget = &widgets[widgetIndex];
            auto& gameState = getGameState();
            switch (widgetIndex)
            {
                case WIDX_STAFF_SPEED_DROPDOWN_BUTTON:
                {
                    Widget* dropdownWidget;

                    dropdownWidget = widget - 1;

                    for (size_t i = 0; i < std::size(_staffSpeedNames); i++)
                    {
                        gDropdownItems[i].Args = _staffSpeedNames[i];
                        gDropdownItems[i].Format = STR_DROPDOWN_MENU_LABEL;
                    }

                    WindowDropdownShowTextCustomWidth(
                        { windowPos.x + dropdownWidget->left, windowPos.y + dropdownWidget->top }, dropdownWidget->height() + 1,
                        colours[1], 0, Dropdown::Flag::StayOpen, 3, dropdownWidget->width() - 3);
                    Dropdown::SetChecked(EnumValue(gameState.cheats.selectedStaffSpeed), true);
                }
            }
        }

        void OnMouseDownWeather(WidgetIndex widgetIndex)
        {
            auto* widget = &widgets[widgetIndex];
            auto& gameState = getGameState();
            switch (widgetIndex)
            {
                case WIDX_WEATHER_DROPDOWN_BUTTON:
                {
                    Widget* dropdownWidget = widget - 1;

                    for (size_t i = 0; i < std::size(WeatherTypes); i++)
                    {
                        gDropdownItems[i].Format = STR_DROPDOWN_MENU_LABEL;
                        gDropdownItems[i].Args = WeatherTypes[i];
                    }
                    WindowDropdownShowTextCustomWidth(
                        { windowPos.x + dropdownWidget->left, windowPos.y + dropdownWidget->top }, dropdownWidget->height() + 1,
                        colours[1], 0, Dropdown::Flag::StayOpen, std::size(WeatherTypes), dropdownWidget->width() - 3);

                    auto currentWeather = gameState.weatherCurrent.weatherType;
                    Dropdown::SetChecked(EnumValue(currentWeather), true);

                    break;
                }
            }
        }

        void OnMouseUpPark(WidgetIndex widgetIndex)
        {
            auto& gameState = getGameState();
            switch (widgetIndex)
            {
                case WIDX_OWN_ALL_LAND:
                    CheatsSet(CheatType::OwnAllLand);
                    break;
                case WIDX_REMOVE_PARK_FENCES:
                    CheatsSet(CheatType::RemoveParkFences);
                    break;
                case WIDX_OPEN_CLOSE_PARK:
                    CheatsSet(CheatType::OpenClosePark);
                    break;
                case WIDX_WIN_SCENARIO:
                    CheatsSet(CheatType::WinScenario);
                    break;
                case WIDX_HAVE_FUN:
                    CheatsSet(CheatType::HaveFun);
                    break;
                case WIDX_NEVERENDING_MARKETING:
                    CheatsSet(CheatType::NeverendingMarketing, !gameState.cheats.neverendingMarketing);
                    break;
                case WIDX_FORCE_PARK_RATING:
                    if (Park::GetForcedRating() >= 0)
                    {
                        CheatsSet(CheatType::SetForcedParkRating, -1);
                    }
                    else
                    {
                        CheatsSet(CheatType::SetForcedParkRating, _parkRatingSpinnerValue);
                    }
                    break;
                case WIDX_ALLOW_BUILD_IN_PAUSE_MODE:
                    CheatsSet(CheatType::BuildInPauseMode, !gameState.cheats.buildInPauseMode);
                    break;
                case WIDX_ALLOW_REGULAR_PATH_AS_QUEUE:
                    CheatsSet(CheatType::AllowRegularPathAsQueue, !gameState.cheats.allowRegularPathAsQueue);
                    break;
                case WIDX_ALLOW_SPECIAL_COLOUR_SCHEMES:
                    CheatsSet(CheatType::AllowSpecialColourSchemes, !gameState.cheats.allowSpecialColourSchemes);
                    break;
            }
        }

        void OnMouseUpStaff(WidgetIndex widgetIndex)
        {
            auto& gameState = getGameState();
            switch (widgetIndex)
            {
                case WIDX_CLEAR_GRASS:
                    CheatsSet(CheatType::SetGrassLength, GRASS_LENGTH_CLEAR_0);
                    break;
                case WIDX_MOWED_GRASS:
                    CheatsSet(CheatType::SetGrassLength, GRASS_LENGTH_MOWED);
                    break;
                case WIDX_WATER_PLANTS:
                    CheatsSet(CheatType::WaterPlants);
                    break;
                case WIDX_FIX_VANDALISM:
                    CheatsSet(CheatType::FixVandalism);
                    break;
                case WIDX_REMOVE_LITTER:
                    CheatsSet(CheatType::RemoveLitter);
                    break;
                case WIDX_DISABLE_PLANT_AGING:
                    CheatsSet(CheatType::DisablePlantAging, !gameState.cheats.disablePlantAging);
                    break;
            }
        }

        void OnMouseUpWeather(WidgetIndex widgetIndex)
        {
            auto& gameState = getGameState();
            switch (widgetIndex)
            {
                case WIDX_FREEZE_WEATHER:
                    CheatsSet(CheatType::FreezeWeather, !gameState.cheats.freezeWeather);
                    break;
                case WIDX_CREATE_DUCKS:
                    CheatsSet(CheatType::CreateDucks, kCheatsDuckIncrement);
                    break;
                case WIDX_REMOVE_DUCKS:
                    CheatsSet(CheatType::RemoveDucks);
                    break;
            }
        }

        void OnDropdownStaff(WidgetIndex widgetIndex, int32_t dropdownIndex)
        {
            if (dropdownIndex == -1)
            {
                return;
            }

            if (widgetIndex == WIDX_STAFF_SPEED_DROPDOWN_BUTTON)
            {
                int32_t speed = kCheatsStaffNormalSpeed;
                auto& gameState = getGameState();
                switch (dropdownIndex)
                {
                    case 0:
                        gameState.cheats.selectedStaffSpeed = StaffSpeedCheat::None;
                        speed = kCheatsStaffNormalSpeed;
                        break;

                    case 1:
                        gameState.cheats.selectedStaffSpeed = StaffSpeedCheat::Frozen;
                        speed = kCheatsStaffFreezeSpeed;
                        break;

                    case 2:
                        gameState.cheats.selectedStaffSpeed = StaffSpeedCheat::Fast;
                        speed = kCheatsStaffFastSpeed;
                }
                CheatsSet(CheatType::SetStaffSpeed, speed);
            }
        }

        void OnDropdownWeather(WidgetIndex widgetIndex, int32_t dropdownIndex)
        {
            if (dropdownIndex == -1)
            {
                return;
            }

            if (widgetIndex == WIDX_WEATHER_DROPDOWN_BUTTON)
            {
                CheatsSet(CheatType::ForceWeather, dropdownIndex);
            }
        }

        void OnMouseUpGuests(WidgetIndex widgetIndex)
        {
            auto& gameState = getGameState();
            switch (widgetIndex)
            {
                case WIDX_GUEST_HAPPINESS_MAX:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_HAPPINESS, kPeepMaxHappiness);
                    break;
                case WIDX_GUEST_HAPPINESS_MIN:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_HAPPINESS, 0);
                    break;
                case WIDX_GUEST_ENERGY_MAX:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_ENERGY, kPeepMaxEnergy);
                    break;
                case WIDX_GUEST_ENERGY_MIN:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_ENERGY, kPeepMinEnergy);
                    break;
                case WIDX_GUEST_HUNGER_MAX:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_HUNGER, 0);
                    break;
                case WIDX_GUEST_HUNGER_MIN:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_HUNGER, kPeepMaxHunger);
                    break;
                case WIDX_GUEST_THIRST_MAX:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_THIRST, 0);
                    break;
                case WIDX_GUEST_THIRST_MIN:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_THIRST, kPeepMaxThirst);
                    break;
                case WIDX_GUEST_NAUSEA_MAX:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_NAUSEA, kPeepMaxNausea);
                    break;
                case WIDX_GUEST_NAUSEA_MIN:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_NAUSEA, 0);
                    break;
                case WIDX_GUEST_NAUSEA_TOLERANCE_MAX:
                    CheatsSet(
                        CheatType::SetGuestParameter, GUEST_PARAMETER_NAUSEA_TOLERANCE, EnumValue(PeepNauseaTolerance::High));
                    break;
                case WIDX_GUEST_NAUSEA_TOLERANCE_MIN:
                    CheatsSet(
                        CheatType::SetGuestParameter, GUEST_PARAMETER_NAUSEA_TOLERANCE, EnumValue(PeepNauseaTolerance::None));
                    break;
                case WIDX_GUEST_TOILET_MAX:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_TOILET, kPeepMaxToilet);
                    break;
                case WIDX_GUEST_TOILET_MIN:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_TOILET, 0);
                    break;
                case WIDX_GUEST_RIDE_INTENSITY_MORE_THAN_1:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_PREFERRED_RIDE_INTENSITY, 1);
                    break;
                case WIDX_GUEST_RIDE_INTENSITY_LESS_THAN_15:
                    CheatsSet(CheatType::SetGuestParameter, GUEST_PARAMETER_PREFERRED_RIDE_INTENSITY, 0);
                    break;
                case WIDX_TRAM_GUESTS:
                    CheatsSet(CheatType::GenerateGuests, kCheatsTramIncrement);
                    break;
                case WIDX_REMOVE_ALL_GUESTS:
                    CheatsSet(CheatType::RemoveAllGuests);
                    break;
                case WIDX_GIVE_GUESTS_MONEY:
                    CheatsSet(CheatType::GiveAllGuests, OBJECT_MONEY);
                    break;
                case WIDX_GIVE_GUESTS_PARK_MAPS:
                    CheatsSet(CheatType::GiveAllGuests, OBJECT_PARK_MAP);
                    break;
                case WIDX_GIVE_GUESTS_BALLOONS:
                    CheatsSet(CheatType::GiveAllGuests, OBJECT_BALLOON);
                    break;
                case WIDX_GIVE_GUESTS_UMBRELLAS:
                    CheatsSet(CheatType::GiveAllGuests, OBJECT_UMBRELLA);
                    break;
                case WIDX_GUEST_IGNORE_RIDE_INTENSITY:
                    CheatsSet(CheatType::IgnoreRideIntensity, !gameState.cheats.ignoreRideIntensity);
                    break;
                case WIDX_GUEST_IGNORE_PRICE:
                    CheatsSet(CheatType::IgnorePrice, !gameState.cheats.ignorePrice);
                    break;
                case WIDX_DISABLE_VANDALISM:
                    CheatsSet(CheatType::DisableVandalism, !gameState.cheats.disableVandalism);
                    break;
                case WIDX_DISABLE_LITTERING:
                    CheatsSet(CheatType::DisableLittering, !gameState.cheats.disableLittering);
                    break;
            }
        }

        void OnMouseUpRides(WidgetIndex widgetIndex)
        {
            auto& gameState = getGameState();
            switch (widgetIndex)
            {
                case WIDX_RENEW_RIDES:
                    CheatsSet(CheatType::RenewRides);
                    break;
                case WIDX_MAKE_DESTRUCTIBLE:
                    CheatsSet(CheatType::MakeDestructible, !gameState.cheats.makeAllDestructible);
                    break;
                case WIDX_FIX_ALL:
                    CheatsSet(CheatType::FixRides);
                    break;
                case WIDX_UNLOCK_OPERATING_LIMITS:
                    CheatsSet(CheatType::FastLiftHill, !gameState.cheats.unlockOperatingLimits);
                    break;
                case WIDX_DISABLE_BRAKES_FAILURE:
                    CheatsSet(CheatType::DisableBrakesFailure, !gameState.cheats.disableBrakesFailure);
                    break;
                case WIDX_DISABLE_ALL_BREAKDOWNS:
                    CheatsSet(CheatType::DisableAllBreakdowns, !gameState.cheats.disableAllBreakdowns);
                    break;
                case WIDX_RESET_CRASH_STATUS:
                    CheatsSet(CheatType::ResetCrashStatus);
                    break;
                case WIDX_10_MINUTE_INSPECTIONS:
                    CheatsSet(CheatType::TenMinuteInspections);
                    break;
                case WIDX_SHOW_ALL_OPERATING_MODES:
                {
                    if (!gameState.cheats.showAllOperatingModes)
                    {
                        ContextShowError(STR_WARNING_IN_CAPS, STR_THIS_FEATURE_IS_CURRENTLY_UNSTABLE, {});
                    }
                    CheatsSet(CheatType::ShowAllOperatingModes, !gameState.cheats.showAllOperatingModes);
                }
                break;
                case WIDX_SHOW_VEHICLES_FROM_OTHER_TRACK_TYPES:
                {
                    if (!gameState.cheats.showVehiclesFromOtherTrackTypes)
                    {
                        ContextShowError(STR_WARNING_IN_CAPS, STR_THIS_FEATURE_IS_CURRENTLY_UNSTABLE, {});
                    }
                    CheatsSet(CheatType::ShowVehiclesFromOtherTrackTypes, !gameState.cheats.showVehiclesFromOtherTrackTypes);
                }
                break;
                case WIDX_DISABLE_TRAIN_LENGTH_LIMITS:
                {
                    if (!gameState.cheats.disableTrainLengthLimit)
                    {
                        ContextShowError(STR_WARNING_IN_CAPS, STR_THIS_FEATURE_IS_CURRENTLY_UNSTABLE, {});
                    }
                    CheatsSet(CheatType::DisableTrainLengthLimit, !gameState.cheats.disableTrainLengthLimit);
                }
                break;
                case WIDX_ENABLE_CHAIN_LIFT_ON_ALL_TRACK:
                    CheatsSet(CheatType::EnableChainLiftOnAllTrack, !gameState.cheats.enableChainLiftOnAllTrack);
                    break;
                case WIDX_ENABLE_ARBITRARY_RIDE_TYPE_CHANGES:
                {
                    if (!gameState.cheats.allowArbitraryRideTypeChanges)
                    {
                        ContextShowError(STR_WARNING_IN_CAPS, STR_THIS_FEATURE_IS_CURRENTLY_UNSTABLE, {});
                    }
                    CheatsSet(CheatType::AllowArbitraryRideTypeChanges, !gameState.cheats.allowArbitraryRideTypeChanges);
                }
                break;
                case WIDX_DISABLE_RIDE_VALUE_AGING:
                    CheatsSet(CheatType::DisableRideValueAging, !gameState.cheats.disableRideValueAging);
                    break;
                case WIDX_IGNORE_RESEARCH_STATUS:
                    CheatsSet(CheatType::IgnoreResearchStatus, !gameState.cheats.ignoreResearchStatus);
                    break;
                case WIDX_ENABLE_ALL_DRAWABLE_TRACK_PIECES:
                    CheatsSet(CheatType::EnableAllDrawableTrackPieces, !gameState.cheats.enableAllDrawableTrackPieces);
                    break;
                case WIDX_ALLOW_TRACK_PLACE_INVALID_HEIGHTS:
                {
                    if (!gameState.cheats.allowTrackPlaceInvalidHeights)
                    {
                        ContextShowError(STR_WARNING_IN_CAPS, STR_THIS_FEATURE_IS_CURRENTLY_UNSTABLE, {});
                    }
                    CheatsSet(CheatType::AllowTrackPlaceInvalidHeights, !gameState.cheats.allowTrackPlaceInvalidHeights);
                }
                break;
                case WIDX_ALLOW_INCOMPLETE_RIDES:
                    CheatsSet(CheatType::AllowIncompleteRides, !gameState.cheats.allowIncompleteRides);
                    break;
                case WIDX_NORMALIZE_RIDE_CRASHES:
                    CheatsSet(CheatType::NormalizeRideCrashes, !gameState.cheats.normalizeRideCrashes);
                    break;
            }
        }
    };

    WindowBase* CheatsOpen()
    {
        auto* windowMgr = GetWindowManager();
        auto* window = windowMgr->BringToFrontByClass(WindowClass::Cheats);
        if (window == nullptr)
        {
            window = windowMgr->Create<CheatsWindow>(WindowClass::Cheats, ScreenCoordsXY(32, 32), kWindowSize);
        }
        return window;
    }
} // namespace OpenRCT2::Ui::Windows
