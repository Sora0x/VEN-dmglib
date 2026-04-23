#pragma once

#include <sdk.hpp>

#include <cstdint>

#include "ven_dmglib/mitigation.hpp"

namespace venplus::damage
{

    struct shadowflame
    {
        static constexpr int   ITEM_ID      = 4645;
        static constexpr float AMP          = 0.20f;
        static constexpr float HP_THRESHOLD = 0.40f;
    };

    struct collector
    {
        static constexpr int   ITEM_ID           = 6676;
        static constexpr float EXECUTE_THRESHOLD = 0.05f;
    };

    namespace detail
    {

        inline bool source_has_item( game_object* source, int item_id )
        {
            if ( source == nullptr ) return false;
            int unused{ 0 };
            return source->has_item( item_id, &unused );
        }

    }

    inline float shadowflame_amp( game_object* source, game_object* target, damage_type type )
    {
        if ( target == nullptr ) return 1.f;
        if ( type == damage_type::physical ) return 1.f;
        if ( !detail::source_has_item( source, shadowflame::ITEM_ID ) ) return 1.f;

        const float target_hp_ratio = target->get_hp() / target->get_max_hp();
        if ( target_hp_ratio >= shadowflame::HP_THRESHOLD ) return 1.f;

        return 1.f + shadowflame::AMP;
    }

    inline bool source_owns_collector( game_object* source )
    {
        return detail::source_has_item( source, collector::ITEM_ID );
    }

    inline bool collector_would_execute( float post_mitigation_damage, game_object* source, game_object* target )
    {
        if ( target == nullptr ) return false;
        if ( !source_owns_collector( source ) ) return false;

        const float hp_after  = target->get_hp() + target->get_all_shield() - post_mitigation_damage;
        const float threshold = target->get_max_hp() * collector::EXECUTE_THRESHOLD;
        return hp_after > 0.f && hp_after <= threshold;
    }

}
