/*****************************************************************************
 * Copyright (c) 2014-2025 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#ifdef ENABLE_SCRIPTING

    #include "../interface/Widget.h"
    #include "CustomListView.h"
    #include "CustomWindow.h"
    #include "ScViewport.hpp"

    #include <memory>
    #include <openrct2/Context.h>
    #include <openrct2/scripting/Duktape.hpp>
    #include <openrct2/scripting/IconNames.hpp>
    #include <openrct2/scripting/ScriptEngine.h>
    #include <openrct2/ui/WindowManager.h>

namespace OpenRCT2::Scripting
{
    class ScWindow;
    class ScWidget
    {
    protected:
        WindowClass _class{};
        rct_windownumber _number{};
        WidgetIndex _widgetIndex{};

    public:
        ScWidget(WindowClass c, rct_windownumber n, WidgetIndex widgetIndex)
            : _class(c)
            , _number(n)
            , _widgetIndex(widgetIndex)
        {
        }

        static DukValue ToDukValue(duk_context* ctx, WindowBase* w, WidgetIndex widgetIndex);

    private:
        std::shared_ptr<ScWindow> window_get() const;

        std::string name_get() const
        {
            auto w = GetWindow();
            if (w != nullptr && IsCustomWindow())
            {
                return OpenRCT2::Ui::Windows::GetWidgetName(w, _widgetIndex);
            }
            return {};
        }

        void name_set(const std::string& value)
        {
            auto w = GetWindow();
            if (w != nullptr && IsCustomWindow())
            {
                OpenRCT2::Ui::Windows::SetWidgetName(w, _widgetIndex, value);
            }
        }

        std::string type_get() const
        {
            auto widget = GetWidget();
            if (widget != nullptr)
            {
                switch (widget->type)
                {
                    case WidgetType::frame:
                        return "frame";
                    case WidgetType::resize:
                        return "resize";
                    case WidgetType::imgBtn:
                    case WidgetType::trnBtn:
                    case WidgetType::flatBtn:
                    case WidgetType::button:
                    case WidgetType::closeBox:
                        return "button";
                    case WidgetType::colourBtn:
                        return "colourpicker";
                    case WidgetType::tab:
                        return "tab";
                    case WidgetType::labelCentred:
                    case WidgetType::label:
                        return "label";
                    case WidgetType::tableHeader:
                        return "table_header";
                    case WidgetType::spinner:
                        return "spinner";
                    case WidgetType::dropdownMenu:
                        return "dropdown";
                    case WidgetType::viewport:
                        return "viewport";
                    case WidgetType::groupbox:
                        return "groupbox";
                    case WidgetType::caption:
                        return "caption";
                    case WidgetType::scroll:
                        return "listview";
                    case WidgetType::checkbox:
                        return "checkbox";
                    case WidgetType::textBox:
                        return "textbox";
                    case WidgetType::empty:
                        return "empty";
                    case WidgetType::placeholder:
                        return "placeholder";
                    case WidgetType::progressBar:
                        return "progress_bar";
                    case WidgetType::horizontalSeparator:
                        return "horizontal_separator";
                    case WidgetType::custom:
                        return "custom";
                }
            }
            return "unknown";
        }

        int32_t x_get() const
        {
            auto widget = GetWidget();
            if (widget != nullptr)
            {
                return widget->left;
            }
            return 0;
        }
        void x_set(int32_t value)
        {
            auto widget = GetWidget();
            if (widget != nullptr)
            {
                auto delta = value - widget->left;

                Invalidate();
                widget->left += delta;
                widget->right += delta;

                if (widget->type == WidgetType::dropdownMenu)
                {
                    auto buttonWidget = widget + 1;
                    buttonWidget->left += delta;
                    buttonWidget->right += delta;
                }
                else if (widget->type == WidgetType::spinner)
                {
                    auto upWidget = widget + 1;
                    upWidget->left += delta;
                    upWidget->right += delta;
                    auto downWidget = widget + 2;
                    downWidget->left += delta;
                    downWidget->right += delta;
                }

                Invalidate(widget);
            }
        }

        int32_t y_get() const
        {
            auto widget = GetWidget();
            if (widget != nullptr)
            {
                auto w = GetWindow();
                return widget->top - w->getTitleBarDiffNormal();
            }
            return 0;
        }
        void y_set(int32_t value)
        {
            auto widget = GetWidget();
            if (widget != nullptr)
            {
                auto w = GetWindow();
                value += w->getTitleBarDiffNormal();
                auto delta = value - widget->top;

                Invalidate();
                widget->top += delta;
                widget->bottom += delta;

                if (widget->type == WidgetType::dropdownMenu)
                {
                    auto buttonWidget = widget + 1;
                    buttonWidget->top += delta;
                    buttonWidget->bottom += delta;
                }
                else if (widget->type == WidgetType::spinner)
                {
                    auto upWidget = widget + 1;
                    upWidget->top += delta;
                    upWidget->bottom += delta;
                    auto downWidget = widget + 2;
                    downWidget->top += delta;
                    downWidget->bottom += delta;
                }

                Invalidate(widget);
            }
        }

        int32_t width_get() const
        {
            auto widget = GetWidget();
            if (widget != nullptr)
            {
                return widget->width() + 1;
            }
            return 0;
        }
        void width_set(int32_t value)
        {
            auto widget = GetWidget();
            if (widget != nullptr)
            {
                auto delta = widget->left + value - (widget->right + 1);

                Invalidate();
                widget->right += delta;

                if (widget->type == WidgetType::dropdownMenu)
                {
                    auto buttonWidget = widget + 1;
                    buttonWidget->left += delta;
                    buttonWidget->right += delta;
                }
                else if (widget->type == WidgetType::spinner)
                {
                    auto upWidget = widget + 1;
                    upWidget->left += delta;
                    upWidget->right += delta;
                    auto downWidget = widget + 2;
                    downWidget->left += delta;
                    downWidget->right += delta;
                }

                Invalidate(widget);
            }
        }

        int32_t height_get() const
        {
            auto widget = GetWidget();
            if (widget != nullptr)
            {
                return widget->height() + 1;
            }
            return 0;
        }
        void height_set(int32_t value)
        {
            auto widget = GetWidget();
            if (widget != nullptr)
            {
                auto delta = widget->top + value - (widget->bottom + 1);

                Invalidate();
                widget->bottom += delta;

                if (widget->type == WidgetType::dropdownMenu)
                {
                    auto buttonWidget = widget + 1;
                    buttonWidget->bottom += delta;
                }
                else if (widget->type == WidgetType::spinner)
                {
                    auto upWidget = widget + 1;
                    upWidget->bottom += delta;
                    auto downWidget = widget + 2;
                    downWidget->bottom += delta;
                }

                Invalidate(widget);
            }
        }

        std::string tooltip_get() const
        {
            auto w = GetWindow();
            if (w != nullptr && IsCustomWindow())
            {
                return OpenRCT2::Ui::Windows::GetWidgetTooltip(w, _widgetIndex);
            }
            return {};
        }
        void tooltip_set(const std::string& value)
        {
            auto w = GetWindow();
            if (w != nullptr && IsCustomWindow())
            {
                OpenRCT2::Ui::Windows::SetWidgetTooltip(w, _widgetIndex, value);
            }
        }

        bool isDisabled_get() const
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                return Ui::widgetIsDisabled(*w, _widgetIndex);
            }
            return false;
        }
        void isDisabled_set(bool value)
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                Ui::widgetSetDisabled(*w, _widgetIndex, value);

                auto widget = GetWidget();
                if (widget != nullptr)
                {
                    if (widget->type == WidgetType::dropdownMenu)
                    {
                        Ui::widgetSetDisabled(*w, _widgetIndex + 1, value);
                    }
                    else if (widget->type == WidgetType::spinner)
                    {
                        Ui::widgetSetDisabled(*w, _widgetIndex + 1, value);
                        Ui::widgetSetDisabled(*w, _widgetIndex + 2, value);
                    }
                }
                Invalidate(widget);
            }
        }

        bool isVisible_get() const
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                return Ui::widgetIsVisible(*w, _widgetIndex);
            }
            return false;
        }
        void isVisible_set(bool value)
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                Ui::widgetSetVisible(*w, _widgetIndex, value);

                auto widget = GetWidget();
                if (widget != nullptr)
                {
                    if (widget->type == WidgetType::dropdownMenu)
                    {
                        Ui::widgetSetVisible(*w, _widgetIndex + 1, value);
                    }
                    else if (widget->type == WidgetType::spinner)
                    {
                        Ui::widgetSetVisible(*w, _widgetIndex + 1, value);
                        Ui::widgetSetVisible(*w, _widgetIndex + 2, value);
                    }
                }
                Invalidate(widget);
            }
        }

    protected:
        std::string text_get() const
        {
            if (IsCustomWindow())
            {
                auto widget = GetWidget();
                if (widget != nullptr && (widget->flags.has(WidgetFlag::textIsString)) && widget->string != nullptr)
                {
                    return widget->string;
                }
            }
            return {};
        }

        void text_set(std::string value)
        {
            auto w = GetWindow();
            if (w != nullptr && IsCustomWindow())
            {
                OpenRCT2::Ui::Windows::UpdateWidgetText(w, _widgetIndex, value);
            }
        }

    public:
        static void Register(duk_context* ctx);

    protected:
        WindowBase* GetWindow() const
        {
            if (_class == WindowClass::MainWindow)
                return WindowGetMain();

            auto* windowMgr = Ui::GetWindowManager();
            return windowMgr->FindByNumber(_class, _number);
        }

        Widget* GetWidget() const
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                return &w->widgets[_widgetIndex];
            }
            return nullptr;
        }

        bool IsCustomWindow() const
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                return w->classification == WindowClass::Custom;
            }
            return false;
        }

        void Invalidate(const Widget* widget)
        {
            if (widget != nullptr)
            {
                auto* windowMgr = Ui::GetWindowManager();
                if (widget->type == WidgetType::dropdownMenu)
                {
                    windowMgr->InvalidateWidgetByNumber(_class, _number, _widgetIndex + 1);
                }
                else if (widget->type == WidgetType::spinner)
                {
                    windowMgr->InvalidateWidgetByNumber(_class, _number, _widgetIndex + 1);
                    windowMgr->InvalidateWidgetByNumber(_class, _number, _widgetIndex + 2);
                }
            }
            Invalidate();
        }

        void Invalidate()
        {
            auto* windowMgr = Ui::GetWindowManager();
            windowMgr->InvalidateWidgetByNumber(_class, _number, _widgetIndex);
        }
    };

    class ScButtonWidget : public ScWidget
    {
    public:
        ScButtonWidget(WindowClass c, rct_windownumber n, WidgetIndex widgetIndex)
            : ScWidget(c, n, widgetIndex)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_set_base_class<ScWidget, ScButtonWidget>(ctx);
            dukglue_register_property(ctx, &ScButtonWidget::border_get, &ScButtonWidget::border_set, "border");
            dukglue_register_property(ctx, &ScButtonWidget::isPressed_get, &ScButtonWidget::isPressed_set, "isPressed");
            dukglue_register_property(ctx, &ScButtonWidget::image_get, &ScButtonWidget::image_set, "image");
            // Explicit template due to text being a base method
            dukglue_register_property<ScButtonWidget, std::string, std::string>(
                ctx, &ScButtonWidget::text_get, &ScButtonWidget::text_set, "text");
        }

    private:
        bool border_get() const
        {
            auto widget = GetWidget();
            if (widget != nullptr)
            {
                return widget->type == WidgetType::imgBtn;
            }
            return false;
        }
        void border_set(bool value)
        {
            auto widget = GetWidget();
            if (widget != nullptr && (widget->type == WidgetType::flatBtn || widget->type == WidgetType::imgBtn))
            {
                if (value)
                    widget->type = WidgetType::imgBtn;
                else
                    widget->type = WidgetType::flatBtn;
                Invalidate();
            }
        }

        bool isPressed_get() const
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                return Ui::widgetIsPressed(*w, _widgetIndex);
            }
            return false;
        }
        void isPressed_set(bool value)
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                Ui::widgetSetCheckboxValue(*w, _widgetIndex, value ? 1 : 0);
                Invalidate();
            }
        }

        uint32_t image_get() const
        {
            auto widget = GetWidget();
            if (widget != nullptr && (widget->type == WidgetType::flatBtn || widget->type == WidgetType::imgBtn))
            {
                if (GetTargetAPIVersion() <= kApiVersionG2Reorder)
                {
                    return LegacyIconIndex(widget->image.GetIndex());
                }
                return widget->image.GetIndex();
            }
            return 0;
        }

        void image_set(DukValue value)
        {
            auto widget = GetWidget();
            if (widget != nullptr && (widget->type == WidgetType::flatBtn || widget->type == WidgetType::imgBtn))
            {
                widget->image = ImageId(ImageFromDuk(value));
                Invalidate();
            }
        }
    };

    class ScCheckBoxWidget : public ScWidget
    {
    public:
        ScCheckBoxWidget(WindowClass c, rct_windownumber n, WidgetIndex widgetIndex)
            : ScWidget(c, n, widgetIndex)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_set_base_class<ScWidget, ScCheckBoxWidget>(ctx);
            dukglue_register_property(ctx, &ScCheckBoxWidget::isChecked_get, &ScCheckBoxWidget::isChecked_set, "isChecked");
            // Explicit template due to text being a base method
            dukglue_register_property<ScCheckBoxWidget, std::string, std::string>(
                ctx, &ScCheckBoxWidget::text_get, &ScCheckBoxWidget::text_set, "text");
        }

    private:
        bool isChecked_get() const
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                return Ui::widgetIsPressed(*w, _widgetIndex);
            }
            return false;
        }
        void isChecked_set(bool value)
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                Ui::widgetSetCheckboxValue(*w, _widgetIndex, value ? 1 : 0);
                Invalidate();
            }
        }
    };

    class ScColourPickerWidget : public ScWidget
    {
    public:
        ScColourPickerWidget(WindowClass c, rct_windownumber n, WidgetIndex widgetIndex)
            : ScWidget(c, n, widgetIndex)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_set_base_class<ScWidget, ScColourPickerWidget>(ctx);
            dukglue_register_property(ctx, &ScColourPickerWidget::colour_get, &ScColourPickerWidget::colour_set, "colour");
        }

    private:
        colour_t colour_get() const
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                return GetWidgetColour(w, _widgetIndex);
            }
            return COLOUR_BLACK;
        }
        void colour_set(colour_t value)
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                UpdateWidgetColour(w, _widgetIndex, value);
                Invalidate();
            }
        }
    };

    class ScDropdownWidget : public ScWidget
    {
    public:
        ScDropdownWidget(WindowClass c, rct_windownumber n, WidgetIndex widgetIndex)
            : ScWidget(c, n, widgetIndex)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_set_base_class<ScWidget, ScDropdownWidget>(ctx);
            dukglue_register_property(ctx, &ScDropdownWidget::items_get, &ScDropdownWidget::items_set, "items");
            dukglue_register_property(
                ctx, &ScDropdownWidget::selectedIndex_get, &ScDropdownWidget::selectedIndex_set, "selectedIndex");
            // Explicit template due to text being a base method
            dukglue_register_property<ScDropdownWidget, std::string, std::string>(
                ctx, &ScDropdownWidget::text_get, &ScDropdownWidget::text_set, "text");
        }

    private:
        int32_t selectedIndex_get() const
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                return GetWidgetSelectedIndex(w, _widgetIndex);
            }
            return -1;
        }
        void selectedIndex_set(int32_t value)
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                UpdateWidgetSelectedIndex(w, _widgetIndex, value);
            }
        }

        std::vector<std::string> items_get() const
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                return GetWidgetItems(w, _widgetIndex);
            }
            return {};
        }

        void items_set(const std::vector<std::string>& value)
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                UpdateWidgetItems(w, _widgetIndex, value);
            }
        }
    };

    class ScGroupBoxWidget : public ScWidget
    {
    public:
        ScGroupBoxWidget(WindowClass c, rct_windownumber n, WidgetIndex widgetIndex)
            : ScWidget(c, n, widgetIndex)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_set_base_class<ScWidget, ScGroupBoxWidget>(ctx);
            // Explicit template due to text being a base method
            dukglue_register_property<ScGroupBoxWidget, std::string, std::string>(
                ctx, &ScGroupBoxWidget::text_get, &ScGroupBoxWidget::text_set, "text");
        }
    };

    class ScLabelWidget : public ScWidget
    {
    public:
        ScLabelWidget(WindowClass c, rct_windownumber n, WidgetIndex widgetIndex)
            : ScWidget(c, n, widgetIndex)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_set_base_class<ScWidget, ScLabelWidget>(ctx);
            // Explicit template due to text being a base method
            dukglue_register_property<ScLabelWidget, std::string, std::string>(
                ctx, &ScLabelWidget::text_get, &ScLabelWidget::text_set, "text");
            dukglue_register_property(ctx, &ScLabelWidget::textAlign_get, &ScLabelWidget::textAlign_set, "textAlign");
        }

    private:
        std::string textAlign_get() const
        {
            auto* widget = GetWidget();
            if (widget != nullptr)
            {
                if (widget->type == WidgetType::labelCentred)
                {
                    return "centred";
                }
            }
            return "left";
        }

        void textAlign_set(const std::string& value)
        {
            auto* widget = GetWidget();
            if (widget != nullptr)
            {
                if (value == "centred")
                    widget->type = WidgetType::labelCentred;
                else
                    widget->type = WidgetType::label;
            }
        }
    };

    class ScListViewWidget : public ScWidget
    {
    public:
        ScListViewWidget(WindowClass c, rct_windownumber n, WidgetIndex widgetIndex)
            : ScWidget(c, n, widgetIndex)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_set_base_class<ScWidget, ScListViewWidget>(ctx);
            dukglue_register_property(ctx, &ScListViewWidget::canSelect_get, &ScListViewWidget::canSelect_set, "canSelect");
            dukglue_register_property(ctx, &ScListViewWidget::isStriped_get, &ScListViewWidget::isStriped_set, "isStriped");
            dukglue_register_property(ctx, &ScListViewWidget::scrollbars_get, &ScListViewWidget::scrollbars_set, "scrollbars");
            dukglue_register_property(
                ctx, &ScListViewWidget::showColumnHeaders_get, &ScListViewWidget::showColumnHeaders_set, "showColumnHeaders");
            dukglue_register_property(ctx, &ScListViewWidget::highlightedCell_get, nullptr, "highlightedCell");
            dukglue_register_property(
                ctx, &ScListViewWidget::selectedCell_get, &ScListViewWidget::selectedCell_set, "selectedCell");
            dukglue_register_property(ctx, &ScListViewWidget::columns_get, &ScListViewWidget::columns_set, "columns");
            dukglue_register_property(ctx, &ScListViewWidget::items_get, &ScListViewWidget::items_set, "items");
        }

    private:
        bool canSelect_get() const
        {
            auto listView = GetListView();
            if (listView != nullptr)
            {
                return listView->CanSelect;
            }
            return false;
        }

        void canSelect_set(bool value)
        {
            auto listView = GetListView();
            if (listView != nullptr)
            {
                listView->CanSelect = value;
            }
        }

        bool isStriped_get() const
        {
            auto listView = GetListView();
            if (listView != nullptr)
            {
                return listView->IsStriped;
            }
            return false;
        }

        void isStriped_set(bool value)
        {
            auto listView = GetListView();
            if (listView != nullptr)
            {
                listView->IsStriped = value;
            }
        }

        DukValue scrollbars_get() const
        {
            auto ctx = GetContext()->GetScriptEngine().GetContext();
            auto scrollType = ScrollbarType::None;
            auto listView = GetListView();
            if (listView != nullptr)
            {
                scrollType = listView->GetScrollbars();
            }
            return ToDuk(ctx, scrollType);
        }

        void scrollbars_set(const DukValue& value)
        {
            auto listView = GetListView();
            if (listView != nullptr)
            {
                listView->SetScrollbars(FromDuk<ScrollbarType>(value));
            }
        }

        bool showColumnHeaders_get() const
        {
            auto listView = GetListView();
            if (listView != nullptr)
            {
                return listView->ShowColumnHeaders;
            }
            return false;
        }

        void showColumnHeaders_set(bool value)
        {
            auto listView = GetListView();
            if (listView != nullptr)
            {
                listView->ShowColumnHeaders = value;
            }
        }

        DukValue highlightedCell_get()
        {
            auto ctx = GetContext()->GetScriptEngine().GetContext();
            auto listView = GetListView();
            if (listView != nullptr)
            {
                return ToDuk(ctx, listView->LastHighlightedCell);
            }
            return ToDuk(ctx, nullptr);
        }

        DukValue selectedCell_get()
        {
            auto ctx = GetContext()->GetScriptEngine().GetContext();
            auto listView = GetListView();
            if (listView != nullptr)
            {
                return ToDuk(ctx, listView->SelectedCell);
            }
            return ToDuk(ctx, nullptr);
        }

        void selectedCell_set(const DukValue& value)
        {
            auto listView = GetListView();
            if (listView != nullptr)
            {
                listView->SelectedCell = FromDuk<std::optional<RowColumn>>(value);
            }
        }

        std::vector<std::vector<std::string>> items_get()
        {
            std::vector<std::vector<std::string>> result;
            auto listView = GetListView();
            if (listView != nullptr)
            {
                for (const auto& item : listView->GetItems())
                {
                    result.push_back(item.Cells);
                }
            }
            return result;
        }

        void items_set(const DukValue& value)
        {
            auto listView = GetListView();
            if (listView != nullptr)
            {
                listView->SetItems(FromDuk<std::vector<ListViewItem>>(value));
            }
        }

        std::vector<DukValue> columns_get()
        {
            std::vector<DukValue> result;
            auto listView = GetListView();
            if (listView != nullptr)
            {
                auto ctx = GetContext()->GetScriptEngine().GetContext();
                for (const auto& column : listView->GetColumns())
                {
                    result.push_back(ToDuk(ctx, column));
                }
            }
            return result;
        }

        void columns_set(const DukValue& value)
        {
            auto listView = GetListView();
            if (listView != nullptr)
            {
                listView->SetColumns(FromDuk<std::vector<ListViewColumn>>(value));
            }
        }

        CustomListView* GetListView() const
        {
            auto w = GetWindow();
            if (w != nullptr)
            {
                return GetCustomListView(w, _widgetIndex);
            }
            return nullptr;
        }
    };

    class ScSpinnerWidget : public ScWidget
    {
    public:
        ScSpinnerWidget(WindowClass c, rct_windownumber n, WidgetIndex widgetIndex)
            : ScWidget(c, n, widgetIndex)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_set_base_class<ScWidget, ScSpinnerWidget>(ctx);
            // Explicit template due to text being a base method
            dukglue_register_property<ScSpinnerWidget, std::string, std::string>(
                ctx, &ScSpinnerWidget::text_get, &ScSpinnerWidget::text_set, "text");
        }
    };

    class ScTextBoxWidget : public ScWidget
    {
    public:
        ScTextBoxWidget(WindowClass c, rct_windownumber n, WidgetIndex widgetIndex)
            : ScWidget(c, n, widgetIndex)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_set_base_class<ScWidget, ScTextBoxWidget>(ctx);
            dukglue_register_property(ctx, &ScTextBoxWidget::maxLength_get, &ScTextBoxWidget::maxLength_set, "maxLength");
            // Explicit template due to text being a base method
            dukglue_register_property<ScTextBoxWidget, std::string, std::string>(
                ctx, &ScTextBoxWidget::text_get, &ScTextBoxWidget::text_set, "text");
            dukglue_register_method(ctx, &ScTextBoxWidget::focus, "focus");
        }

    private:
        int32_t maxLength_get() const
        {
            auto w = GetWindow();
            if (w != nullptr && IsCustomWindow())
            {
                return OpenRCT2::Ui::Windows::GetWidgetMaxLength(w, _widgetIndex);
            }
            return 0;
        }

        void maxLength_set(int32_t value)
        {
            auto w = GetWindow();
            if (w != nullptr && IsCustomWindow())
            {
                OpenRCT2::Ui::Windows::SetWidgetMaxLength(w, _widgetIndex, value);
            }
        }

        void focus()
        {
            auto w = GetWindow();
            if (w != nullptr && IsCustomWindow())
            {
                WindowStartTextbox(*w, _widgetIndex, GetWidget()->string, Ui::Windows::GetWidgetMaxLength(w, _widgetIndex));
            }
        }
    };

    class ScViewportWidget : public ScWidget
    {
    public:
        ScViewportWidget(WindowClass c, rct_windownumber n, WidgetIndex widgetIndex)
            : ScWidget(c, n, widgetIndex)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_set_base_class<ScWidget, ScViewportWidget>(ctx);
            dukglue_register_property(ctx, &ScViewportWidget::viewport_get, nullptr, "viewport");
        }

    private:
        std::shared_ptr<ScViewport> viewport_get() const
        {
            auto w = GetWindow();
            if (w != nullptr && IsCustomWindow())
            {
                auto widget = GetWidget();
                if (widget != nullptr && widget->type == WidgetType::viewport)
                {
                    return std::make_shared<ScViewport>(w->classification, w->number);
                }
            }
            return {};
        }
    };

    inline DukValue ScWidget::ToDukValue(duk_context* ctx, WindowBase* w, WidgetIndex widgetIndex)
    {
        const auto& widget = w->widgets[widgetIndex];
        auto c = w->classification;
        auto n = w->number;
        switch (widget.type)
        {
            case WidgetType::button:
            case WidgetType::flatBtn:
            case WidgetType::imgBtn:
                return GetObjectAsDukValue(ctx, std::make_shared<ScButtonWidget>(c, n, widgetIndex));
            case WidgetType::checkbox:
                return GetObjectAsDukValue(ctx, std::make_shared<ScCheckBoxWidget>(c, n, widgetIndex));
            case WidgetType::colourBtn:
                return GetObjectAsDukValue(ctx, std::make_shared<ScColourPickerWidget>(c, n, widgetIndex));
            case WidgetType::dropdownMenu:
                return GetObjectAsDukValue(ctx, std::make_shared<ScDropdownWidget>(c, n, widgetIndex));
            case WidgetType::groupbox:
                return GetObjectAsDukValue(ctx, std::make_shared<ScGroupBoxWidget>(c, n, widgetIndex));
            case WidgetType::label:
            case WidgetType::labelCentred:
                return GetObjectAsDukValue(ctx, std::make_shared<ScLabelWidget>(c, n, widgetIndex));
            case WidgetType::scroll:
                return GetObjectAsDukValue(ctx, std::make_shared<ScListViewWidget>(c, n, widgetIndex));
            case WidgetType::spinner:
                return GetObjectAsDukValue(ctx, std::make_shared<ScSpinnerWidget>(c, n, widgetIndex));
            case WidgetType::textBox:
                return GetObjectAsDukValue(ctx, std::make_shared<ScTextBoxWidget>(c, n, widgetIndex));
            case WidgetType::viewport:
                return GetObjectAsDukValue(ctx, std::make_shared<ScViewportWidget>(c, n, widgetIndex));
            default:
                return GetObjectAsDukValue(ctx, std::make_shared<ScWidget>(c, n, widgetIndex));
        }
    }

} // namespace OpenRCT2::Scripting

#endif
