#pragma once

#include <sdk.hpp>

#include <cstdint>

namespace venplus::damage
{

    enum class damage_type : uint8_t
    {
        physical,
        magic,
        true_damage,
    };

    namespace detail
    {

        struct penetration
        {
            float flat{ 0.f };
            float total_percent{ 0.f };
            float bonus_percent{ 0.f };
        };

        inline penetration build_armor_penetration( game_object* source )
        {
            return penetration{
                .flat          = source->get_physical_lethality(),
                .total_percent = 1.f - source->get_percent_armor_penetration(),
                .bonus_percent = 1.f - source->get_percent_bonus_armor_penetration(),
            };
        }

        inline penetration build_magic_penetration( game_object* source )
        {
            return penetration{
                .flat          = source->get_flat_magic_penetration(),
                .total_percent = 1.f - source->get_percent_magic_penetration(),
                .bonus_percent = 1.f - source->get_percent_bonus_magic_penetration(),
            };
        }

        inline float effective_resist( float total, float bonus, const penetration& pen )
        {
            if ( total <= 0.f ) return total;

            const float base            = total - bonus;
            const float base_after_pct  = base  * ( 1.f - pen.total_percent );
            const float bonus_after_pct = bonus * ( 1.f - pen.total_percent ) * ( 1.f - pen.bonus_percent );

            const float effective = base_after_pct + bonus_after_pct - pen.flat;
            return ( effective > 0.f ) ? effective : 0.f;
        }

    }

    inline float resist_multiplier( float resist )
    {
        if ( resist >= 0.f )
            return 100.f / ( 100.f + resist );

        return 2.f - 100.f / ( 100.f - resist );
    }

    inline float mitigate_physical( float raw, game_object* source, game_object* target )
    {
        if ( source == nullptr || target == nullptr ) return raw;

        const float total_armor = target->get_armor();
        const float bonus_armor = target->get_bonus_armor();
        const auto  pen         = detail::build_armor_penetration( source );

        const float effective = detail::effective_resist( total_armor, bonus_armor, pen );
        return raw * resist_multiplier( effective );
    }

    inline float mitigate_magic( float raw, game_object* source, game_object* target )
    {
        if ( source == nullptr || target == nullptr ) return raw;

        const float total_mr = target->get_magic_resist();
        const float bonus_mr = target->get_bonus_magic_resist();
        const auto  pen      = detail::build_magic_penetration( source );

        const float effective = detail::effective_resist( total_mr, bonus_mr, pen );
        return raw * resist_multiplier( effective );
    }

    inline float mitigate( float raw, damage_type type, game_object* source, game_object* target )
    {
        if ( type == damage_type::true_damage ) return raw;
        if ( type == damage_type::physical )    return mitigate_physical( raw, source, target );
        return mitigate_magic( raw, source, target );
    }

}
