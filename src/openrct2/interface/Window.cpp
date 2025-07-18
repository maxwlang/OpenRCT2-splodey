/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Window.h"

#include "../Context.h"
#include "../Diagnostic.h"
#include "../Editor.h"
#include "../Game.h"
#include "../GameState.h"
#include "../Input.h"
#include "../OpenRCT2.h"
#include "../audio/Audio.h"
#include "../config/Config.h"
#include "../drawing/Drawing.h"
#include "../interface/Cursors.h"
#include "../ride/RideAudio.h"
#include "../ui/UiContext.h"
#include "../ui/WindowManager.h"
#include "Viewport.h"
#include "Widget.h"
#include "WindowBase.h"

#include <cassert>
#include <cmath>
#include <functional>
#include <iterator>
#include <list>

namespace OpenRCT2
{

    std::vector<std::unique_ptr<WindowBase>> g_window_list;
    WindowBase* gWindowAudioExclusive;

    WindowCloseModifier gLastCloseModifier = { { WindowClass::Null, 0 }, CloseWindowModifier::none };

    uint32_t gWindowUpdateTicks;
    colour_t gCurrentWindowColours[3];

    Tool gCurrentToolId;
    WidgetRef gCurrentToolWidget;

    // converted from uint16_t values at 0x009A41EC - 0x009A4230
    // these are percentage coordinates of the viewport to centre to, if a window is obscuring a location, the next is tried
    // clang-format off
static constexpr float kWindowScrollLocations[][2] = {
    { 0.5f, 0.5f },
    { 0.75f, 0.5f },
    { 0.25f, 0.5f },
    { 0.5f, 0.75f },
    { 0.5f, 0.25f },
    { 0.75f, 0.75f },
    { 0.75f, 0.25f },
    { 0.25f, 0.75f },
    { 0.25f, 0.25f },
    { 0.125f, 0.5f },
    { 0.875f, 0.5f },
    { 0.5f, 0.125f },
    { 0.5f, 0.875f },
    { 0.875f, 0.125f },
    { 0.875f, 0.875f },
    { 0.125f, 0.875f },
    { 0.125f, 0.125f },
};
    // clang-format on

    static void WindowDrawCore(RenderTarget& rt, WindowBase& w, int32_t left, int32_t top, int32_t right, int32_t bottom);
    static void WindowDrawSingle(RenderTarget& rt, WindowBase& w, int32_t left, int32_t top, int32_t right, int32_t bottom);

    std::vector<std::unique_ptr<WindowBase>>::iterator WindowGetIterator(const WindowBase* w)
    {
        return std::find_if(g_window_list.begin(), g_window_list.end(), [w](auto&& w2) { return w == w2.get(); });
    }

    void WindowVisitEach(std::function<void(WindowBase*)> func)
    {
        for (auto& w : g_window_list)
        {
            if (w->flags & WF_DEAD)
                continue;
            func(w.get());
        }
    }

    void WindowSetFlagForAllViewports(uint32_t viewportFlag, bool enabled)
    {
        WindowVisitEach([&](WindowBase* w) {
            if (w->viewport != nullptr)
            {
                if (enabled)
                    w->viewport->flags |= viewportFlag;
                else
                    w->viewport->flags &= ~viewportFlag;
            }
        });
    }

    /**
     *
     *  rct2: 0x006ED7B0
     */
    void WindowDispatchUpdateAll()
    {
        // gTooltipNotShownTicks++;
        WindowVisitEach([&](WindowBase* w) { w->OnUpdate(); });
    }

    void WindowUpdateAllViewports()
    {
        WindowVisitEach([&](WindowBase* w) {
            if (w->viewport != nullptr && w->isVisible)
            {
                ViewportUpdatePosition(w);
            }
        });
    }

    static void WindowUpdateVisibilities()
    {
        const auto itEnd = g_window_list.end();
        for (auto it = g_window_list.begin(); it != itEnd; ++it)
        {
            auto& window = *(*it);
            if (window.viewport == nullptr)
            {
                window.isVisible = true;
                continue;
            }
            if (window.classification == WindowClass::MainWindow)
            {
                window.isVisible = true;
                window.viewport->isVisible = true;
                continue;
            }
            window.isVisible = true;
            window.viewport->isVisible = true;
            for (auto itOther = std::next(it); itOther != itEnd; ++itOther)
            {
                const auto& otherWindow = *(*itOther);
                if (otherWindow.flags & WF_DEAD)
                    continue;

                if (otherWindow.windowPos.x <= window.windowPos.x && otherWindow.windowPos.y <= window.windowPos.y
                    && otherWindow.windowPos.x + otherWindow.width >= window.windowPos.x + window.width
                    && otherWindow.windowPos.y + otherWindow.height >= window.windowPos.y + window.height)
                {
                    window.isVisible = false;
                    window.viewport->isVisible = false;
                    break;
                }
            }
        }
    }

    /**
     *
     *  rct2: 0x006E77A1
     */
    void WindowUpdateAll()
    {
        // Remove all windows in g_window_list that have the WF_DEAD flag
        g_window_list.erase(
            std::remove_if(g_window_list.begin(), g_window_list.end(), [](auto&& w) -> bool { return w->flags & WF_DEAD; }),
            g_window_list.end());

        // Periodic update happens every second so 40 ticks.
        if (gCurrentRealTimeTicks >= gWindowUpdateTicks)
        {
            gWindowUpdateTicks = gCurrentRealTimeTicks + kGameUpdateFPS;

            WindowVisitEach([](WindowBase* w) { w->OnPeriodicUpdate(); });
        }

        // Border flash invalidation
        WindowVisitEach([](WindowBase* w) {
            if (w->flags & WF_WHITE_BORDER_MASK)
            {
                w->flags -= WF_WHITE_BORDER_ONE;
                if (!(w->flags & WF_WHITE_BORDER_MASK))
                {
                    w->Invalidate();
                }
            }
        });

        auto windowManager = Ui::GetWindowManager();
        windowManager->UpdateMouseWheel();

        WindowUpdateVisibilities();
    }

    void WindowNotifyLanguageChange()
    {
        WindowVisitEach([&](WindowBase* w) { w->OnLanguageChange(); });
    }

    /*
     * Changes the maximum amount of windows allowed
     */
    void WindowSetWindowLimit(int32_t value)
    {
        int32_t prev = Config::Get().general.WindowLimit;
        int32_t val = std::clamp<int32_t>(value, kWindowLimitMin, kWindowLimitMax);
        Config::Get().general.WindowLimit = val;
        Config::Save();
        // Checks if value decreases and then closes surplus
        // windows if one sets a limit lower than the number of windows open
        if (val < prev)
        {
            auto* windowMgr = Ui::GetWindowManager();
            windowMgr->CloseSurplus(val, WindowClass::Options);
        }
    }

    int32_t WindowGetScrollDataIndex(const WindowBase& w, WidgetIndex widget_index)
    {
        int32_t i, result;

        result = 0;
        for (i = 0; i < widget_index; i++)
        {
            const auto& widget = w.widgets[i];
            if (widget.type == WidgetType::scroll)
                result++;
        }
        return result;
    }

    /**
     *
     *  rct2: 0x006EE65A
     */
    void WindowPushOthersRight(WindowBase& window)
    {
        WindowVisitEach([&window](WindowBase* w) {
            if (w == &window)
                return;
            if (w->flags & (WF_STICK_TO_BACK | WF_STICK_TO_FRONT))
                return;
            if (w->windowPos.x >= window.windowPos.x + window.width)
                return;
            if (w->windowPos.x + w->width <= window.windowPos.x)
                return;
            if (w->windowPos.y >= window.windowPos.y + window.height)
                return;
            if (w->windowPos.y + w->height <= window.windowPos.y)
                return;

            w->Invalidate();
            if (window.windowPos.x + window.width + 13 >= ContextGetWidth())
                return;
            auto push_amount = window.windowPos.x + window.width - w->windowPos.x + 3;
            w->windowPos.x += push_amount;
            w->Invalidate();
            if (w->viewport != nullptr)
                w->viewport->pos.x += push_amount;
        });
    }

    /**
     *
     *  rct2: 0x006EE6EA
     */
    void WindowPushOthersBelow(WindowBase& w1)
    {
        // Enumerate through all other windows
        WindowVisitEach([&w1](WindowBase* w2) {
            if (&w1 == w2)
                return;
            // ?
            if (w2->flags & (WF_STICK_TO_BACK | WF_STICK_TO_FRONT))
                return;
            // Check if w2 intersects with w1
            if (w2->windowPos.x > (w1.windowPos.x + w1.width) || w2->windowPos.x + w2->width < w1.windowPos.x)
                return;
            if (w2->windowPos.y > (w1.windowPos.y + w1.height) || w2->windowPos.y + w2->height < w1.windowPos.y)
                return;

            // Check if there is room to push it down
            if (w1.windowPos.y + w1.height + 80 >= ContextGetHeight())
                return;

            // Invalidate the window's current area
            w2->Invalidate();

            int32_t push_amount = w1.windowPos.y + w1.height - w2->windowPos.y + 3;
            w2->windowPos.y += push_amount;

            // Invalidate the window's new area
            w2->Invalidate();

            // Update viewport position if necessary
            if (w2->viewport != nullptr)
                w2->viewport->pos.y += push_amount;
        });
    }

    /**
     *
     *  rct2: 0x006EE2E4
     */
    WindowBase* WindowGetMain()
    {
        for (auto& w : g_window_list)
        {
            if (w->flags & WF_DEAD)
                continue;
            if (w->classification == WindowClass::MainWindow)
            {
                return w.get();
            }
        }
        return nullptr;
    }

    /**
     *
     *  rct2: 0x006E7C9C
     * @param w (esi)
     * @param x (eax)
     * @param y (ecx)
     * @param z (edx)
     */
    void WindowScrollToLocation(WindowBase& w, const CoordsXYZ& coords)
    {
        WindowUnfollowSprite(w);

        if (w.viewport == nullptr)
        {
            return;
        }

        int16_t height = TileElementHeight(coords);
        if (coords.z < height - 16)
        {
            if (!(w.viewport->flags & VIEWPORT_FLAG_UNDERGROUND_INSIDE))
            {
                w.viewport->flags |= VIEWPORT_FLAG_UNDERGROUND_INSIDE;
                w.Invalidate();
            }
        }
        else
        {
            if (w.viewport->flags & VIEWPORT_FLAG_UNDERGROUND_INSIDE)
            {
                w.viewport->flags &= ~VIEWPORT_FLAG_UNDERGROUND_INSIDE;
                w.Invalidate();
            }
        }

        auto screenCoords = Translate3DTo2DWithZ(w.viewport->rotation, coords);

        int32_t i = 0;
        if (gLegacyScene != LegacyScene::titleSequence)
        {
            bool found = false;
            while (!found)
            {
                auto x2 = w.viewport->pos.x + static_cast<int32_t>(w.viewport->width * kWindowScrollLocations[i][0]);
                auto y2 = w.viewport->pos.y + static_cast<int32_t>(w.viewport->height * kWindowScrollLocations[i][1]);

                auto it = WindowGetIterator(&w);
                for (; it != g_window_list.end(); it++)
                {
                    if ((*it)->flags & WF_DEAD)
                        continue;

                    auto w2 = (*it).get();
                    auto x1 = w2->windowPos.x - 10;
                    auto y1 = w2->windowPos.y - 10;
                    if (x2 >= x1 && x2 <= w2->width + x1 + 20)
                    {
                        if (y2 >= y1 && y2 <= w2->height + y1 + 20)
                        {
                            // window is covering this area, try the next one
                            i++;
                            found = false;
                            break;
                        }
                    }
                }
                if (it == g_window_list.end())
                {
                    found = true;
                }
                if (i >= static_cast<int32_t>(std::size(kWindowScrollLocations)))
                {
                    i = 0;
                    found = true;
                }
            }
        }

        // rct2: 0x006E7C76
        if (w.viewport_target_sprite.IsNull())
        {
            if (!(w.flags & WF_NO_SCROLLING))
            {
                w.savedViewPos = screenCoords
                    - ScreenCoordsXY{ static_cast<int32_t>(w.viewport->ViewWidth() * kWindowScrollLocations[i][0]),
                                      static_cast<int32_t>(w.viewport->ViewHeight() * kWindowScrollLocations[i][1]) };
                w.flags |= WF_SCROLLING_TO_LOCATION;
            }
        }
    }

    void WindowViewportGetMapCoordsByCursor(
        const WindowBase& w, int32_t* map_x, int32_t* map_y, int32_t* offset_x, int32_t* offset_y)
    {
        // Get mouse position to offset against.
        auto mouseCoords = ContextGetCursorPositionScaled();

        // Compute map coordinate by mouse position.
        auto viewportPos = w.viewport->ScreenToViewportCoord(mouseCoords);
        auto coordsXYZ = ViewportAdjustForMapHeight(viewportPos, w.viewport->rotation);
        auto mapCoords = ViewportPosToMapPos(viewportPos, coordsXYZ.z, w.viewport->rotation);
        *map_x = mapCoords.x;
        *map_y = mapCoords.y;

        // Get viewport coordinates centring around the tile.
        int32_t z = TileElementHeight(mapCoords);

        auto centreLoc = centre_2d_coordinates({ mapCoords.x, mapCoords.y, z }, w.viewport);
        if (!centreLoc)
        {
            LOG_ERROR("Invalid location.");
            return;
        }

        // Rebase mouse position onto centre of window, and compensate for zoom level.
        int32_t rebased_x = w.viewport->zoom.ApplyTo(w.width / 2 - mouseCoords.x);
        int32_t rebased_y = w.viewport->zoom.ApplyTo(w.height / 2 - mouseCoords.y);

        // Compute cursor offset relative to tile.
        *offset_x = w.viewport->zoom.ApplyTo(w.savedViewPos.x - (centreLoc->x + rebased_x));
        *offset_y = w.viewport->zoom.ApplyTo(w.savedViewPos.y - (centreLoc->y + rebased_y));
    }

    void WindowViewportCentreTileAroundCursor(WindowBase& w, int32_t map_x, int32_t map_y, int32_t offset_x, int32_t offset_y)
    {
        // Get viewport coordinates centring around the tile.
        int32_t z = TileElementHeight({ map_x, map_y });
        auto centreLoc = centre_2d_coordinates({ map_x, map_y, z }, w.viewport);

        if (!centreLoc.has_value())
        {
            LOG_ERROR("Invalid location.");
            return;
        }

        // Get mouse position to offset against.
        auto mouseCoords = ContextGetCursorPositionScaled();

        // Rebase mouse position onto centre of window, and compensate for zoom level.
        int32_t rebased_x = w.viewport->zoom.ApplyTo((w.width >> 1) - mouseCoords.x);
        int32_t rebased_y = w.viewport->zoom.ApplyTo((w.height >> 1) - mouseCoords.y);

        // Apply offset to the viewport.
        w.savedViewPos = { centreLoc->x + rebased_x + w.viewport->zoom.ApplyInversedTo(offset_x),
                           centreLoc->y + rebased_y + w.viewport->zoom.ApplyInversedTo(offset_y) };
    }

    /**
     * For all windows with viewports, ensure they do not have a zoom level less than the minimum.
     */
    void WindowCheckAllValidZoom()
    {
        WindowVisitEach([](WindowBase* w) {
            if (w->viewport != nullptr && w->viewport->zoom < ZoomLevel::min())
            {
                WindowZoomSet(*w, ZoomLevel::min(), false);
            }
        });
    }

    void WindowZoomSet(WindowBase& w, ZoomLevel zoomLevel, bool atCursor)
    {
        Viewport* v = w.viewport;
        if (v == nullptr)
            return;

        zoomLevel = std::clamp(zoomLevel, ZoomLevel::min(), ZoomLevel::max());
        if (v->zoom == zoomLevel)
            return;

        // Zooming to cursor? Remember where we're pointing at the moment.
        int32_t saved_map_x = 0;
        int32_t saved_map_y = 0;
        int32_t offset_x = 0;
        int32_t offset_y = 0;
        if (Config::Get().general.ZoomToCursor && atCursor)
        {
            WindowViewportGetMapCoordsByCursor(w, &saved_map_x, &saved_map_y, &offset_x, &offset_y);
        }

        // Zoom in
        while (v->zoom > zoomLevel)
        {
            v->zoom--;
            w.savedViewPos.x += v->ViewWidth() / 2;
            w.savedViewPos.y += v->ViewHeight() / 2;
        }

        // Zoom out
        while (v->zoom < zoomLevel)
        {
            v->zoom++;
            w.savedViewPos.x -= v->ViewWidth() / 4;
            w.savedViewPos.y -= v->ViewHeight() / 4;
        }

        // Zooming to cursor? Centre around the tile we were hovering over just now.
        if (Config::Get().general.ZoomToCursor && atCursor)
        {
            WindowViewportCentreTileAroundCursor(w, saved_map_x, saved_map_y, offset_x, offset_y);
        }

        // HACK: Prevents the redraw from failing when there is
        // a window on top of the viewport.
        auto* windowMgr = Ui::GetWindowManager();
        windowMgr->BringToFront(w);
        w.Invalidate();
    }

    /**
     * Splits a drawing of a window into regions that can be seen and are not hidden
     * by other opaque overlapping windows.
     */
    void WindowDraw(RenderTarget& rt, WindowBase& w, int32_t left, int32_t top, int32_t right, int32_t bottom)
    {
        if (!w.isVisible)
            return;

        // Divide the draws up for only the visible regions of the window recursively
        auto itPos = WindowGetIterator(&w);
        for (auto it = std::next(itPos); it != g_window_list.end(); it++)
        {
            // Check if this window overlaps w
            auto topwindow = it->get();
            if (topwindow->flags & WF_TRANSPARENT)
                continue;
            if (topwindow->flags & WF_DEAD)
                continue;
            if (topwindow->windowPos.x >= right || topwindow->windowPos.y >= bottom)
                continue;
            if (topwindow->windowPos.x + topwindow->width <= left || topwindow->windowPos.y + topwindow->height <= top)
                continue;

            // A window overlaps w, split up the draw into two regions where the window starts to overlap
            if (topwindow->windowPos.x > left)
            {
                // Split draw at topwindow.left
                WindowDrawCore(rt, w, left, top, topwindow->windowPos.x, bottom);
                WindowDrawCore(rt, w, topwindow->windowPos.x, top, right, bottom);
            }
            else if (topwindow->windowPos.x + topwindow->width < right)
            {
                // Split draw at topwindow.right
                WindowDrawCore(rt, w, left, top, topwindow->windowPos.x + topwindow->width, bottom);
                WindowDrawCore(rt, w, topwindow->windowPos.x + topwindow->width, top, right, bottom);
            }
            else if (topwindow->windowPos.y > top)
            {
                // Split draw at topwindow.top
                WindowDrawCore(rt, w, left, top, right, topwindow->windowPos.y);
                WindowDrawCore(rt, w, left, topwindow->windowPos.y, right, bottom);
            }
            else if (topwindow->windowPos.y + topwindow->height < bottom)
            {
                // Split draw at topwindow.bottom
                WindowDrawCore(rt, w, left, top, right, topwindow->windowPos.y + topwindow->height);
                WindowDrawCore(rt, w, left, topwindow->windowPos.y + topwindow->height, right, bottom);
            }

            // Drawing for this region should be done now, exit
            return;
        }

        // No windows overlap
        WindowDrawCore(rt, w, left, top, right, bottom);
    }

    /**
     * Draws the given window and any other overlapping transparent windows.
     */
    static void WindowDrawCore(RenderTarget& rt, WindowBase& w, int32_t left, int32_t top, int32_t right, int32_t bottom)
    {
        // Clamp region
        left = std::max<int32_t>(left, w.windowPos.x);
        top = std::max<int32_t>(top, w.windowPos.y);
        right = std::min<int32_t>(right, w.windowPos.x + w.width);
        bottom = std::min<int32_t>(bottom, w.windowPos.y + w.height);
        if (left >= right)
            return;
        if (top >= bottom)
            return;

        // Draw the window and any other overlapping transparent windows
        for (auto it = WindowGetIterator(&w); it != g_window_list.end(); it++)
        {
            auto* v = (*it).get();
            if (v->flags & WF_DEAD)
                continue;
            if ((&w == v || (v->flags & WF_TRANSPARENT)) && v->isVisible)
            {
                WindowDrawSingle(rt, *v, left, top, right, bottom);
            }
        }
    }

    static void WindowDrawSingle(RenderTarget& rt, WindowBase& w, int32_t left, int32_t top, int32_t right, int32_t bottom)
    {
        assert(rt.zoom_level == ZoomLevel{ 0 });
        // Copy render target so we can crop it
        RenderTarget copy = rt;

        // Clamp left to 0
        int32_t overflow = left - copy.x;
        if (overflow > 0)
        {
            copy.x += overflow;
            copy.width -= overflow;
            if (copy.width <= 0)
                return;
            copy.pitch += overflow;
            copy.bits += overflow;
        }

        // Clamp width to right
        overflow = copy.x + copy.width - right;
        if (overflow > 0)
        {
            copy.width -= overflow;
            if (copy.width <= 0)
                return;
            copy.pitch += overflow;
        }

        // Clamp top to 0
        overflow = top - copy.y;
        if (overflow > 0)
        {
            copy.y += overflow;
            copy.height -= overflow;
            if (copy.height <= 0)
                return;
            copy.bits += copy.LineStride() * overflow;
        }

        // Clamp height to bottom
        overflow = copy.y + copy.height - bottom;
        if (overflow > 0)
        {
            copy.height -= overflow;
            if (copy.height <= 0)
                return;
        }

        // Invalidate modifies the window colours so first get the correct
        // colour before setting the global variables for the string painting
        w.OnPrepareDraw();

        // Text colouring
        gCurrentWindowColours[0] = w.colours[0].colour;
        gCurrentWindowColours[1] = w.colours[1].colour;
        gCurrentWindowColours[2] = w.colours[2].colour;

        w.OnDraw(copy);
    }

    bool isToolActive(WindowClass cls)
    {
        return gInputFlags.has(InputFlag::toolActive) && gCurrentToolWidget.window_classification == cls;
    }

    bool isToolActive(WindowClass cls, rct_windownumber number)
    {
        return isToolActive(cls) && gCurrentToolWidget.window_number == number;
    }

    bool isToolActive(WindowClass cls, WidgetIndex widgetIndex)
    {
        return isToolActive(cls) && gCurrentToolWidget.widget_index == widgetIndex;
    }

    bool isToolActive(WindowClass cls, WidgetIndex widgetIndex, rct_windownumber number)
    {
        return isToolActive(cls, widgetIndex) && gCurrentToolWidget.window_number == number;
    }

    bool isToolActive(const WindowBase& w, WidgetIndex widgetIndex)
    {
        return isToolActive(w.classification, widgetIndex, w.number);
    }

    /**
     *
     *  rct2: 0x006EE212
     *
     * @param tool (al)
     * @param widgetIndex (dx)
     * @param w (esi)
     */
    bool ToolSet(const WindowBase& w, WidgetIndex widgetIndex, Tool tool)
    {
        if (gInputFlags.has(InputFlag::toolActive))
        {
            if (w.classification == gCurrentToolWidget.window_classification && w.number == gCurrentToolWidget.window_number
                && widgetIndex == gCurrentToolWidget.widget_index)
            {
                ToolCancel();
                return true;
            }

            ToolCancel();
        }

        gInputFlags.set(InputFlag::toolActive);
        gInputFlags.unset(InputFlag::unk4);
        gInputFlags.unset(InputFlag::unk6);
        gCurrentToolId = tool;
        gCurrentToolWidget.window_classification = w.classification;
        gCurrentToolWidget.window_number = w.number;
        gCurrentToolWidget.widget_index = widgetIndex;
        return false;
    }

    /**
     *
     *  rct2: 0x006EE281
     */
    void ToolCancel()
    {
        if (gInputFlags.has(InputFlag::toolActive))
        {
            gInputFlags.unset(InputFlag::toolActive);

            MapInvalidateSelectionRect();
            MapInvalidateMapSelectionTiles();

            // Reset map selection
            gMapSelectFlags = 0;

            if (gCurrentToolWidget.widget_index != kWidgetIndexNull)
            {
                auto* windowMgr = Ui::GetWindowManager();

                // Invalidate tool widget
                windowMgr->InvalidateWidgetByNumber(
                    gCurrentToolWidget.window_classification, gCurrentToolWidget.window_number,
                    gCurrentToolWidget.widget_index);

                // Abort tool event
                WindowBase* w = windowMgr->FindByNumber(
                    gCurrentToolWidget.window_classification, gCurrentToolWidget.window_number);
                if (w != nullptr)
                    w->OnToolAbort(gCurrentToolWidget.widget_index);
            }
        }
    }

    /**
     * rct2: 0x0066B905
     */
    void WindowResizeGui(int32_t width, int32_t height)
    {
        WindowResizeGuiScenarioEditor(width, height);
        if (isInEditorMode())
            return;

        auto* windowMgr = Ui::GetWindowManager();
        WindowBase* titleWind = windowMgr->FindByClass(WindowClass::TitleMenu);
        if (titleWind != nullptr)
        {
            titleWind->windowPos.x = (width - titleWind->width) / 2;
            titleWind->windowPos.y = height - 182;
        }

        WindowBase* versionWind = windowMgr->FindByClass(WindowClass::TitleVersion);
        if (versionWind != nullptr)
            versionWind->windowPos.y = height - 30;

        WindowBase* exitWind = windowMgr->FindByClass(WindowClass::TitleExit);
        if (exitWind != nullptr)
        {
            exitWind->windowPos.x = width - 40;
            exitWind->windowPos.y = height - 64;
        }

        WindowBase* optionsWind = windowMgr->FindByClass(WindowClass::TitleOptions);
        if (optionsWind != nullptr)
        {
            optionsWind->windowPos.x = width - 80;
        }

        // Keep options window centred after a resize
        WindowBase* optionsWindow = windowMgr->FindByClass(WindowClass::Options);
        if (optionsWindow != nullptr)
        {
            optionsWindow->windowPos.x = (ContextGetWidth() - optionsWindow->width) / 2;
            optionsWindow->windowPos.y = (ContextGetHeight() - optionsWindow->height) / 2;
        }

        // Keep progress bar window centred after a resize
        WindowBase* ProgressWindow = windowMgr->FindByClass(WindowClass::ProgressWindow);
        if (ProgressWindow != nullptr)
        {
            ProgressWindow->windowPos.x = (ContextGetWidth() - ProgressWindow->width) / 2;
            ProgressWindow->windowPos.y = (ContextGetHeight() - ProgressWindow->height) / 2;
        }

        GfxInvalidateScreen();
    }

    /**
     * rct2: 0x0066F0DD
     */
    void WindowResizeGuiScenarioEditor(int32_t width, int32_t height)
    {
        auto* mainWind = WindowGetMain();
        if (mainWind != nullptr)
        {
            Viewport* viewport = mainWind->viewport;
            mainWind->width = width;
            mainWind->height = height;
            viewport->width = width;
            viewport->height = height;
            if (!mainWind->widgets.empty() && mainWind->widgets[WC_MAIN_WINDOW__0].type == WidgetType::viewport)
            {
                mainWind->widgets[WC_MAIN_WINDOW__0].right = width;
                mainWind->widgets[WC_MAIN_WINDOW__0].bottom = height;
            }
        }

        auto* windowMgr = Ui::GetWindowManager();

        WindowBase* topWind = windowMgr->FindByClass(WindowClass::TopToolbar);
        if (topWind != nullptr)
        {
            topWind->width = std::max(640, width);
        }

        WindowBase* bottomWind = windowMgr->FindByClass(WindowClass::BottomToolbar);
        if (bottomWind != nullptr)
        {
            bottomWind->windowPos.y = height - 32;
            bottomWind->width = std::max(640, width);
        }
    }

    /**
     * Update zoom based volume attenuation for ride music and clear music list.
     *  rct2: 0x006BC348
     */
    void WindowUpdateViewportRideMusic()
    {
        RideAudio::ClearAllViewportInstances();
        g_music_tracking_viewport = nullptr;

        for (auto it = g_window_list.rbegin(); it != g_window_list.rend(); it++)
        {
            auto w = it->get();
            auto viewport = w->viewport;
            if (viewport == nullptr || !(viewport->flags & VIEWPORT_FLAG_SOUND_ON))
                continue;

            g_music_tracking_viewport = viewport;
            gWindowAudioExclusive = w;

            if (viewport->zoom <= ZoomLevel{ 0 })
                Audio::gVolumeAdjustZoom = 0;
            else if (viewport->zoom == ZoomLevel{ 1 })
                Audio::gVolumeAdjustZoom = 30;
            else
                Audio::gVolumeAdjustZoom = 60;
            break;
        }
    }

    /**
     *
     *  rct2: 0x006EE3C3
     */
    void TextinputCancel()
    {
        auto* windowMgr = Ui::GetWindowManager();
        windowMgr->CloseByClass(WindowClass::Textinput);
    }

    /**
     *
     *  rct2: 0x006E7499
     * left (ax)
     * top (bx)
     * right (dx)
     * bottom (bp)
     */
    void WindowDrawAll(RenderTarget& rt, int32_t left, int32_t top, int32_t right, int32_t bottom)
    {
        auto windowRT = rt.Crop({ left, top }, { right - left, bottom - top });
        WindowVisitEach([&windowRT, left, top, right, bottom](WindowBase* w) {
            if (w->flags & WF_TRANSPARENT)
                return;
            if (right <= w->windowPos.x || bottom <= w->windowPos.y)
                return;
            if (left >= w->windowPos.x + w->width || top >= w->windowPos.y + w->height)
                return;
            WindowDraw(windowRT, *w, left, top, right, bottom);
        });
    }

    void WindowInitAll()
    {
        auto* windowMgr = Ui::GetWindowManager();
        windowMgr->CloseAllExceptFlags(0);
    }

    void WindowFollowSprite(WindowBase& w, EntityId spriteIndex)
    {
        if (spriteIndex.ToUnderlying() < kMaxEntities || spriteIndex.IsNull())
        {
            w.viewport_smart_follow_sprite = spriteIndex;
        }
    }

    void WindowUnfollowSprite(WindowBase& w)
    {
        w.viewport_smart_follow_sprite = EntityId::GetNull();
        w.viewport_target_sprite = EntityId::GetNull();
    }

    Viewport* WindowGetViewport(WindowBase* w)
    {
        if (w == nullptr)
        {
            return nullptr;
        }

        return w->viewport;
    }

    // TODO: declared in WindowManager.h; move when refactors continue
    Ui::IWindowManager* Ui::GetWindowManager()
    {
        return GetContext()->GetUiContext().GetWindowManager();
    }
} // namespace OpenRCT2
