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

    #include "ScEntity.hpp"

struct MoneyEffect;

namespace OpenRCT2::Scripting
{

    class ScMoneyEffect : public ScEntity
    {
    public:
        ScMoneyEffect(EntityId Id);

        static void Register(duk_context* ctx);

    private:
        MoneyEffect* GetMoneyEffect() const;

        money64 value_get() const;
        void value_set(money64);
    };

} // namespace OpenRCT2::Scripting
#endif
