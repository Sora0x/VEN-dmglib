#pragma once

#include <sdk.hpp>

#include <cstdint>

namespace venplus::damage
{

    struct cut_down
    {
        static constexpr uint32_t ID           = 8017u;
        static constexpr float    AMP          = 0.08f;
        static constexpr float    HP_THRESHOLD = 0.60f;
    };

    struct coup_de_grace
    {
        static constexpr uint32_t ID           = 8014u;
        static constexpr float    AMP          = 0.08f;
        static constexpr float    HP_THRESHOLD = 0.40f;
    };

    struct last_stand
    {
        static constexpr uint32_t ID       = 8299u;
        static constexpr float    AMP_MIN  = 0.05f;
        static constexpr float    AMP_MAX  = 0.11f;
        static constexpr float    HP_START = 0.60f;
        static constexpr float    HP_MAX   = 0.30f;
    };

    struct axiom_arcanist
    {
        static constexpr uint32_t ID      = 8473u;
        static constexpr float    AMP     = 0.12f;
        static constexpr float    AMP_AOE = 0.08f;
    };

    inline float cut_down_amp( game_object* source, game_object* target )
    {
        if ( source == nullptr || target == nullptr ) return 1.f;
        if ( !target->is_hero() )                     return 1.f;
        if ( !source->has_rune( cut_down::ID ) )      return 1.f;

        const float target_hp_ratio = target->get_hp() / target->get_max_hp();
        if ( target_hp_ratio <= cut_down::HP_THRESHOLD ) return 1.f;

        return 1.f + cut_down::AMP;
    }

    inline float coup_de_grace_amp( game_object* source, game_object* target )
    {
        if ( source == nullptr || target == nullptr )  return 1.f;
        if ( !target->is_hero() )                      return 1.f;
        if ( !source->has_rune( coup_de_grace::ID ) )  return 1.f;

        const float target_hp_ratio = target->get_hp() / target->get_max_hp();
        if ( target_hp_ratio >= coup_de_grace::HP_THRESHOLD ) return 1.f;

        return 1.f + coup_de_grace::AMP;
    }

    inline float last_stand_amp( game_object* source, game_object* target )
    {
        if ( source == nullptr || target == nullptr ) return 1.f;
        if ( !target->is_hero() )                     return 1.f;
        if ( !source->has_rune( last_stand::ID ) )    return 1.f;

        const float source_hp_ratio = source->get_hp() / source->get_max_hp();
        if ( source_hp_ratio >= last_stand::HP_START ) return 1.f;

        float t = ( last_stand::HP_START - source_hp_ratio )
                / ( last_stand::HP_START - last_stand::HP_MAX );
        if ( t > 1.f ) t = 1.f;

        const float amp = last_stand::AMP_MIN + t * ( last_stand::AMP_MAX - last_stand::AMP_MIN );
        return 1.f + amp;
    }

    inline float axiom_arcanist_amp( game_object* source, bool is_aoe )
    {
        if ( source == nullptr )                         return 1.f;
        if ( !source->has_rune( axiom_arcanist::ID ) )   return 1.f;

        return 1.f + ( is_aoe ? axiom_arcanist::AMP_AOE : axiom_arcanist::AMP );
    }

}
