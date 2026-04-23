#pragma once

#include <sdk.hpp>

#include <cstdint>
#include <string_view>

namespace venplus::damage
{

    struct baron
    {
        static constexpr uint32_t DEBUFF_HASH      = 0xD7D32CF6u;
        static constexpr float    DAMAGE_REDUCTION = 0.50f;
    };

    struct dragon
    {
        static constexpr uint32_t VENGEANCE_HASH   = 0xEDFDC9u;
        static constexpr float    GRUDGE_PER_STACK = 0.15f;
    };

    struct elder
    {
        static constexpr uint32_t BUFF_HASH         = 0x6B7098B1u;
        static constexpr float    EXECUTE_THRESHOLD = 0.20f;
    };

    namespace detail
    {

        inline bool is_dragon_kill_buff( buff_instance* buff )
        {
            if ( buff == nullptr || !buff->is_active() ) return false;

            const auto name = buff->get_name();
            if ( name.find( "SRX_DragonBuff" ) == std::string_view::npos ) return false;
            if ( name.find( "Soul" )           != std::string_view::npos ) return false;
            if ( name.find( "Preview" )        != std::string_view::npos ) return false;

            return true;
        }

        inline int dragon_buff_stacks( buff_instance* buff )
        {
            const int stacks_a = static_cast< int >( buff->get_stacks() );
            const int stacks_b = buff->get_count();
            const int bigger  = ( stacks_a > stacks_b ) ? stacks_a : stacks_b;
            return ( bigger > 0 ) ? bigger : 1;
        }

    }

    inline bool is_baron_nashor( game_object* target )
    {
        if ( target == nullptr ) return false;

        const auto name = target->get_char_name();
        if ( name == "SRU_Baron" ) return true;
        return name.find( "Baron" ) != std::string_view::npos;
    }

    inline bool is_dragon( game_object* target )
    {
        if ( target == nullptr ) return false;
        if ( !target->is_epic_monster() ) return false;

        const std::string_view name = target->get_char_name();
        return name.find( "dragon" ) != std::string_view::npos
            || name.find( "Dragon" ) != std::string_view::npos;
    }

    inline int count_dragons_killed( game_object* source )
    {
        if ( source == nullptr ) return 0;

        int total = 0;
        for ( auto* buff : source->get_buffs() )
        {
            if ( !detail::is_dragon_kill_buff( buff ) ) continue;
            total += detail::dragon_buff_stacks( buff );
        }
        return total;
    }

    inline bool source_has_elder_buff( game_object* source )
    {
        if ( source == nullptr ) return false;
        auto* buff = source->get_buff_by_hash( elder::BUFF_HASH );
        return buff != nullptr && buff->is_active();
    }

    inline float baron_reduction( float damage, game_object* source, game_object* target )
    {
        if ( source == nullptr || target == nullptr ) return damage;
        if ( !is_baron_nashor( target ) ) return damage;

        auto* debuff = source->get_buff_by_hash( baron::DEBUFF_HASH );
        if ( debuff == nullptr || !debuff->is_active() ) return damage;

        return damage * ( 1.f - baron::DAMAGE_REDUCTION );
    }

    inline float dragon_reduction( float damage, game_object* source, game_object* target )
    {
        if ( source == nullptr || target == nullptr ) return damage;
        if ( !is_dragon( target ) ) return damage;

        auto* vengeance = target->get_buff_by_hash( dragon::VENGEANCE_HASH );
        if ( vengeance == nullptr || !vengeance->is_active() ) return damage;

        const int killed = count_dragons_killed( source );
        if ( killed <= 0 ) return damage;

        const float reduction  = dragon::GRUDGE_PER_STACK * static_cast< float >( killed );
        const float multiplier = 1.f - reduction;
        return damage * ( multiplier > 0.f ? multiplier : 0.f );
    }

    inline bool elder_would_execute( float post_mitigation_damage, game_object* source, game_object* target )
    {
        if ( source == nullptr || target == nullptr ) return false;
        if ( !source_has_elder_buff( source ) ) return false;

        const float hp_after  = target->get_hp() + target->get_all_shield() - post_mitigation_damage;
        const float threshold = target->get_max_hp() * elder::EXECUTE_THRESHOLD;
        return hp_after <= threshold;
    }

}
