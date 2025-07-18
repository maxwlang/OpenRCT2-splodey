/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Windows.h"

#include <algorithm>
#include <openrct2-ui/UiContext.h>
#include <openrct2-ui/input/ShortcutManager.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2/SpriteIds.h>
#include <openrct2/drawing/Drawing.h>
#include <openrct2/localisation/Formatter.h>
#include <openrct2/localisation/StringIds.h>
#include <openrct2/ui/WindowManager.h>

namespace OpenRCT2::Ui::Windows
{
    WindowBase* ResetShortcutKeysPromptOpen();

    static constexpr StringId kWindowTitle = STR_SHORTCUTS_TITLE;
    static constexpr ScreenSize kWindowSize = { 420, 280 };
    static constexpr ScreenSize kMaximumWindowSize = { 1200, 800 };

    enum WindowShortcutWidgetIdx
    {
        WIDX_BACKGROUND,
        WIDX_TITLE,
        WIDX_CLOSE,
        WIDX_TAB_CONTENT_PANEL,
        WIDX_SCROLL,
        WIDX_RESET,
        WIDX_TAB_0,
    };

    // clang-format off
    static constexpr auto _shortcutWidgets = makeWidgets(
        makeWindowShim(kWindowTitle, kWindowSize),
        makeWidget({0,                      43}, {350, 287}, WidgetType::resize, WindowColour::secondary                                                        ),
        makeWidget({4,                      47}, {412, 215}, WidgetType::scroll, WindowColour::primary, SCROLL_VERTICAL,           STR_SHORTCUT_LIST_TIP        ),
        makeWidget({4, kWindowSize.height - 15}, {150,  12}, WidgetType::button, WindowColour::primary, STR_SHORTCUT_ACTION_RESET, STR_SHORTCUT_ACTION_RESET_TIP)
    );
    // clang-format on

    static constexpr StringId kWindowTitleChange = STR_SHORTCUT_CHANGE_TITLE;
    static constexpr ScreenSize kWindowSizeChange = { 250, 80 };

    enum
    {
        WIDX_REMOVE = 3
    };

    // clang-format off
    static constexpr auto window_shortcut_change_widgets = makeWidgets(
        makeWindowShim(kWindowTitleChange, kWindowSizeChange),
        makeWidget({ 75, 56 }, { 100, 14 }, WidgetType::button, WindowColour::primary, STR_SHORTCUT_REMOVE, STR_SHORTCUT_REMOVE_TIP)
    );
    // clang-format on

    class ChangeShortcutWindow final : public Window
    {
    private:
        std::string _shortcutId;
        StringId _shortcutLocalisedName{};
        std::string _shortcutCustomName;

    public:
        static ChangeShortcutWindow* Open(std::string_view shortcutId)
        {
            auto& shortcutManager = GetShortcutManager();
            auto registeredShortcut = shortcutManager.GetShortcut(shortcutId);
            if (registeredShortcut != nullptr)
            {
                auto* windowMgr = GetWindowManager();
                windowMgr->CloseByClass(WindowClass::ChangeKeyboardShortcut);
                auto* w = windowMgr->Create<ChangeShortcutWindow>(
                    WindowClass::ChangeKeyboardShortcut, kWindowSizeChange, WF_CENTRE_SCREEN);
                if (w != nullptr)
                {
                    w->_shortcutId = shortcutId;
                    w->_shortcutLocalisedName = registeredShortcut->LocalisedName;
                    w->_shortcutCustomName = registeredShortcut->CustomName;
                    shortcutManager.SetPendingShortcutChange(registeredShortcut->Id);
                    return w;
                }
            }
            return nullptr;
        }

        void OnOpen() override
        {
            SetWidgets(window_shortcut_change_widgets);
            WindowInitScrollWidgets(*this);
        }

        void OnClose() override
        {
            auto& shortcutManager = GetShortcutManager();
            shortcutManager.SetPendingShortcutChange({});
            NotifyShortcutKeysWindow();
        }

        void OnMouseUp(WidgetIndex widgetIndex) override
        {
            switch (widgetIndex)
            {
                case WIDX_CLOSE:
                    Close();
                    break;
                case WIDX_REMOVE:
                    Remove();
                    break;
            }
        }

        void OnDraw(RenderTarget& rt) override
        {
            DrawWidgets(rt);

            ScreenCoordsXY stringCoords(windowPos.x + 125, windowPos.y + widgets[WIDX_TITLE].bottom + 16);

            auto ft = Formatter();
            if (_shortcutCustomName.empty())
            {
                ft.Add<StringId>(_shortcutLocalisedName);
            }
            else
            {
                ft.Add<StringId>(STR_STRING);
                ft.Add<const char*>(_shortcutCustomName.c_str());
            }
            DrawTextWrapped(rt, stringCoords, 242, STR_SHORTCUT_CHANGE_PROMPT, ft, { TextAlignment::CENTRE });
        }

    private:
        void NotifyShortcutKeysWindow();

        void Remove()
        {
            auto& shortcutManager = GetShortcutManager();
            auto* shortcut = shortcutManager.GetShortcut(_shortcutId);
            if (shortcut != nullptr)
            {
                shortcut->Current.clear();
                shortcutManager.SaveUserBindings();
            }
            Close();
        }
    };

    class ShortcutKeysWindow final : public Window
    {
    private:
        struct ShortcutStringPair
        {
            std::string ShortcutId;
            ::StringId StringId = kStringIdNone;
            std::string CustomString;
            std::string Binding;
        };

        struct ShortcutTabDesc
        {
            std::string_view IdGroup;
            uint32_t ImageId;
            uint32_t ImageDivisor;
            uint32_t ImageNumFrames;
        };

        std::vector<ShortcutTabDesc> _tabs;
        std::vector<Widget> _widgets;
        std::vector<ShortcutStringPair> _list;
        int_fast16_t _highlightedItem;
        size_t _currentTabIndex{};
        uint32_t _tabAnimationIndex{};

    public:
        void OnOpen() override
        {
            InitialiseTabs();
            InitialiseWidgets();
            InitialiseList();
        }

        void OnClose() override
        {
            auto* windowMgr = Ui::GetWindowManager();
            windowMgr->CloseByClass(WindowClass::ResetShortcutKeysPrompt);
        }

        void OnResize() override
        {
            WindowSetResize(*this, kWindowSize, kMaximumWindowSize);
        }

        void OnUpdate() override
        {
            // Remove highlight when the mouse is not hovering over the list
            if (_highlightedItem != -1 && !widgetIsHighlighted(*this, WIDX_SCROLL))
            {
                _highlightedItem = -1;
                InvalidateWidget(WIDX_SCROLL);
            }

            _tabAnimationIndex++;
            InvalidateWidget(static_cast<WidgetIndex>(WIDX_TAB_0 + _currentTabIndex));
        }

        void OnMouseUp(WidgetIndex widgetIndex) override
        {
            switch (widgetIndex)
            {
                case WIDX_CLOSE:
                    Close();
                    break;
                case WIDX_RESET:
                    ResetShortcutKeysPromptOpen();
                    break;
                default:
                {
                    auto tabIndex = static_cast<size_t>(widgetIndex - WIDX_TAB_0);
                    if (tabIndex < _tabs.size())
                    {
                        SetTab(tabIndex);
                    }
                }
            }
        }

        void OnPrepareDraw() override
        {
            widgets[WIDX_SCROLL].right = width - 5;
            widgets[WIDX_SCROLL].bottom = height - 19;
            widgets[WIDX_RESET].top = height - 16;
            widgets[WIDX_RESET].bottom = height - 5;
            WindowAlignTabs(this, WIDX_TAB_0, static_cast<WidgetIndex>(WIDX_TAB_0 + _tabs.size() - 1));

            // Set selected tab
            for (size_t i = 0; i < _tabs.size(); i++)
            {
                SetWidgetPressed(static_cast<WidgetIndex>(WIDX_TAB_0 + i), false);
            }
            SetWidgetPressed(static_cast<WidgetIndex>(WIDX_TAB_0 + _currentTabIndex), true);
        }

        void OnDraw(RenderTarget& rt) override
        {
            DrawWidgets(rt);
            DrawTabImages(rt);
        }

        ScreenSize OnScrollGetSize(int32_t scrollIndex) override
        {
            auto h = static_cast<int32_t>(_list.size() * kScrollableRowHeight);
            auto bottom = std::max(0, h - widgets[WIDX_SCROLL].bottom + widgets[WIDX_SCROLL].top + 21);
            if (bottom < scrolls[0].contentOffsetY)
            {
                scrolls[0].contentOffsetY = bottom;
                Invalidate();
            }
            return { 0, h };
        }

        void OnScrollMouseOver(int32_t scrollIndex, const ScreenCoordsXY& screenCoords) override
        {
            auto index = static_cast<int_fast16_t>((screenCoords.y - 1) / kScrollableRowHeight);
            if (static_cast<size_t>(index) < _list.size())
            {
                _highlightedItem = index;
                Invalidate();
            }
            else
            {
                _highlightedItem = -1;
            }
        }

        void OnScrollMouseDown(int32_t scrollIndex, const ScreenCoordsXY& screenCoords) override
        {
            auto selectedItem = static_cast<size_t>((screenCoords.y - 1) / kScrollableRowHeight);
            if (selectedItem < _list.size())
            {
                // Is this a separator?
                if (!_list[selectedItem].ShortcutId.empty())
                {
                    auto& shortcut = _list[selectedItem];
                    ChangeShortcutWindow::Open(shortcut.ShortcutId);
                }
            }
        }

        void OnScrollDraw(int32_t scrollIndex, RenderTarget& rt) override
        {
            auto rtCoords = ScreenCoordsXY{ rt.x, rt.y };
            GfxFillRect(
                rt, { rtCoords, rtCoords + ScreenCoordsXY{ rt.width - 1, rt.height - 1 } },
                ColourMapA[colours[1].colour].mid_light);

            // TODO: the line below is a workaround for what is presumably a bug with dpi->width
            //       see https://github.com/OpenRCT2/OpenRCT2/issues/11238 for details
            const auto scrollWidth = width - kScrollBarWidth - 10;

            for (size_t i = 0; i < _list.size(); ++i)
            {
                auto y = static_cast<int32_t>(1 + i * kScrollableRowHeight);
                if (y > rt.y + rt.height)
                {
                    break;
                }

                if (y + kScrollableRowHeight < rt.y)
                {
                    continue;
                }

                // Is this a separator?
                if (_list[i].ShortcutId.empty())
                {
                    DrawSeparator(rt, y, scrollWidth);
                }
                else
                {
                    auto isHighlighted = _highlightedItem == static_cast<int_fast16_t>(i);
                    DrawItem(rt, y, scrollWidth, _list[i], isHighlighted);
                }
            }
        }

        void OnLanguageChange() override
        {
            InitialiseList();
        }

        void RefreshBindings()
        {
            InitialiseList();
        }

        void ResetAllOnActiveTab()
        {
            auto& shortcutManager = GetShortcutManager();
            for (const auto& item : _list)
            {
                auto shortcut = shortcutManager.GetShortcut(item.ShortcutId);
                if (shortcut != nullptr)
                {
                    shortcut->Current = shortcut->Default;
                }
            }
            shortcutManager.SaveUserBindings();
            RefreshBindings();
        }

    private:
        bool IsInCurrentTab(const RegisteredShortcut& shortcut)
        {
            auto groupFilter = _tabs[_currentTabIndex].IdGroup;
            auto group = shortcut.GetTopLevelGroup();
            if (groupFilter.empty())
            {
                // Check it doesn't belong in any other tab
                for (const auto& tab : _tabs)
                {
                    if (!tab.IdGroup.empty())
                    {
                        if (tab.IdGroup == group)
                        {
                            return false;
                        }
                    }
                }
                return true;
            }

            return group == groupFilter;
        }

        void InitialiseList()
        {
            // Get shortcuts and sort by group
            auto shortcuts = GetShortcutsForCurrentTab();
            std::stable_sort(shortcuts.begin(), shortcuts.end(), [](const RegisteredShortcut* a, const RegisteredShortcut* b) {
                return a->OrderIndex < b->OrderIndex;
            });

            // Create list items with a separator between each group
            _list.clear();
            std::string group;
            for (const auto* shortcut : shortcuts)
            {
                if (group.empty())
                {
                    group = shortcut->GetGroup();
                }
                else
                {
                    auto groupName = shortcut->GetGroup();
                    if (group != groupName)
                    {
                        // Add separator
                        group = groupName;
                        _list.emplace_back();
                    }
                }

                ShortcutStringPair ssp;
                ssp.ShortcutId = shortcut->Id;
                ssp.StringId = shortcut->LocalisedName;
                ssp.CustomString = shortcut->CustomName;
                ssp.Binding = shortcut->GetDisplayString();
                _list.push_back(std::move(ssp));
            }

            Invalidate();
        }

        std::vector<const RegisteredShortcut*> GetShortcutsForCurrentTab()
        {
            std::vector<const RegisteredShortcut*> result;
            auto& shortcutManager = GetShortcutManager();
            for (const auto& shortcut : shortcutManager.Shortcuts)
            {
                if (IsInCurrentTab(shortcut.second))
                {
                    result.push_back(&shortcut.second);
                }
            }
            return result;
        }

        void InitialiseTabs()
        {
            _tabs.clear();
            _tabs.push_back({ "interface", SPR_TAB_GEARS_0, 2, 4 });
            _tabs.push_back({ "view", SPR_G2_VIEW, 0, 0 });
            _tabs.push_back({ "window", SPR_TAB_PARK_ENTRANCE, 0, 0 });
            _tabs.push_back({ {}, SPR_TAB_WRENCH_0, 2, 16 });
        }

        void InitialiseWidgets()
        {
            widgets.clear();
            widgets.insert(widgets.begin(), std::begin(_shortcutWidgets), std::end(_shortcutWidgets));

            int32_t x = 3;
            for (size_t i = 0; i < _tabs.size(); i++)
            {
                auto tab = makeTab({ x, 17 }, kStringIdNone);
                widgets.push_back(tab);
                x += 31;
            }

            WindowInitScrollWidgets(*this);
            ResizeFrame();
        }

        void SetTab(size_t index)
        {
            if (_currentTabIndex != index)
            {
                _currentTabIndex = index;
                _tabAnimationIndex = 0;
                InitialiseList();
            }
        }

        void DrawTabImages(RenderTarget& rt) const
        {
            for (size_t i = 0; i < _tabs.size(); i++)
            {
                DrawTabImage(rt, i);
            }
        }

        void DrawTabImage(RenderTarget& rt, size_t tabIndex) const
        {
            const auto& tabDesc = _tabs[tabIndex];
            auto widgetIndex = static_cast<WidgetIndex>(WIDX_TAB_0 + tabIndex);
            if (!IsWidgetDisabled(widgetIndex))
            {
                auto imageId = tabDesc.ImageId;
                if (imageId != 0)
                {
                    if (tabIndex == _currentTabIndex && tabDesc.ImageDivisor != 0 && tabDesc.ImageNumFrames != 0)
                    {
                        auto frame = _tabAnimationIndex / tabDesc.ImageDivisor;
                        imageId += frame % tabDesc.ImageNumFrames;
                    }

                    const auto& widget = widgets[widgetIndex];
                    GfxDrawSprite(rt, ImageId(imageId), windowPos + ScreenCoordsXY{ widget.left, widget.top });
                }
            }
        }

        void DrawSeparator(RenderTarget& rt, int32_t y, int32_t scrollWidth)
        {
            const int32_t top = y + (kScrollableRowHeight / 2) - 1;
            GfxFillRect(rt, { { 0, top }, { scrollWidth, top } }, ColourMapA[colours[0].colour].mid_dark);
            GfxFillRect(rt, { { 0, top + 1 }, { scrollWidth, top + 1 } }, ColourMapA[colours[0].colour].lightest);
        }

        void DrawItem(RenderTarget& rt, int32_t y, int32_t scrollWidth, const ShortcutStringPair& shortcut, bool isHighlighted)
        {
            auto format = STR_BLACK_STRING;
            if (isHighlighted)
            {
                format = STR_WINDOW_COLOUR_2_STRINGID;
                GfxFilterRect(rt, { 0, y - 1, scrollWidth, y + (kScrollableRowHeight - 2) }, FilterPaletteID::PaletteDarken1);
            }

            auto bindingOffset = (scrollWidth * 2) / 3;
            auto ft = Formatter();
            ft.Add<StringId>(STR_SHORTCUT_ENTRY_FORMAT);
            if (shortcut.CustomString.empty())
            {
                ft.Add<StringId>(shortcut.StringId);
            }
            else
            {
                ft.Add<StringId>(STR_STRING);
                ft.Add<const char*>(shortcut.CustomString.c_str());
            }
            DrawTextEllipsised(rt, { 0, y - 1 }, bindingOffset, format, ft);

            if (!shortcut.Binding.empty())
            {
                ft = Formatter();
                ft.Add<StringId>(STR_STRING);
                ft.Add<const char*>(shortcut.Binding.c_str());
                DrawTextEllipsised(rt, { bindingOffset, y - 1 }, 150, format, ft);
            }
        }
    };

    void ChangeShortcutWindow::NotifyShortcutKeysWindow()
    {
        auto* windowMgr = GetWindowManager();
        auto w = windowMgr->FindByClass(WindowClass::KeyboardShortcutList);
        if (w != nullptr)
        {
            static_cast<ShortcutKeysWindow*>(w)->RefreshBindings();
        }
    }

    WindowBase* ShortcutKeysOpen()
    {
        auto* windowMgr = GetWindowManager();
        auto w = windowMgr->BringToFrontByClass(WindowClass::KeyboardShortcutList);
        if (w == nullptr)
        {
            w = windowMgr->Create<ShortcutKeysWindow>(WindowClass::KeyboardShortcutList, kWindowSize, WF_RESIZABLE);
        }
        return w;
    }

#pragma region Reset prompt
    static constexpr ScreenSize kWindowSizeReset = { 200, 80 };

    enum
    {
        WIDX_RESET_PROMPT_BACKGROUND,
        WIDX_RESET_PROMPT_TITLE,
        WIDX_RESET_PROMPT_CLOSE,
        WIDX_RESET_PROMPT_LABEL,
        WIDX_RESET_PROMPT_RESET,
        WIDX_RESET_PROMPT_CANCEL
    };

    // clang-format off
    static constexpr auto WindowResetShortcutKeysPromptWidgets = makeWidgets(
        makeWindowShim(STR_SHORTCUT_ACTION_RESET, kWindowSizeReset),
        makeWidget({                           2,                           30 }, { kWindowSizeReset.width - 4, 12 }, WidgetType::labelCentred, WindowColour::primary, STR_RESET_SHORTCUT_KEYS_PROMPT),
        makeWidget({                           8, kWindowSizeReset.height - 22 }, {                         85, 14 }, WidgetType::button,       WindowColour::primary, STR_RESET),
        makeWidget({ kWindowSizeReset.width - 95, kWindowSizeReset.height - 22 }, {                         85, 14 }, WidgetType::button,       WindowColour::primary, STR_SAVE_PROMPT_CANCEL)
    );
    // clang-format on

    class ResetShortcutKeysPrompt final : public Window
    {
        void OnOpen() override
        {
            SetWidgets(WindowResetShortcutKeysPromptWidgets);
        }

        void OnMouseUp(WidgetIndex widgetIndex) override
        {
            switch (widgetIndex)
            {
                case WIDX_RESET_PROMPT_RESET:
                {
                    auto* windowMgr = GetWindowManager();
                    auto w = windowMgr->FindByClass(WindowClass::KeyboardShortcutList);
                    if (w != nullptr)
                    {
                        static_cast<ShortcutKeysWindow*>(w)->ResetAllOnActiveTab();
                    }
                    Close();
                    break;
                }
                case WIDX_RESET_PROMPT_CANCEL:
                case WIDX_RESET_PROMPT_CLOSE:
                    Close();
                    break;
            }
        }
    };

    WindowBase* ResetShortcutKeysPromptOpen()
    {
        auto* windowMgr = GetWindowManager();
        return windowMgr->FocusOrCreate<ResetShortcutKeysPrompt>(
            WindowClass::ResetShortcutKeysPrompt, kWindowSizeReset, WF_CENTRE_SCREEN | WF_TRANSPARENT);
    }
#pragma endregion
} // namespace OpenRCT2::Ui::Windows
