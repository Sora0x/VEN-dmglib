#pragma once

#include <sdk.hpp>

#include <cstdint>

#include "ven_dmglib/mitigation.hpp"
#include "ven_dmglib/runes.hpp"
#include "ven_dmglib/items.hpp"
#include "ven_dmglib/objectives.hpp"

namespace venplus::damage
{

    enum class spell_kind : uint8_t
    {
        normal,
        ultimate_single,
        ultimate_aoe,
    };

    enum class crit_state : uint8_t
    {
        no_crit,
        crit,
    };

    namespace detail
    {

        inline float apply_spell_amps(
            float raw,
            game_object* source,
            game_object* target,
            damage_type  type,
            spell_kind   kind )
        {
            raw *= cut_down_amp(      source, target );
            raw *= coup_de_grace_amp( source, target );
            raw *= last_stand_amp(    source, target );

            if ( kind != spell_kind::normal )
            {
                const bool is_aoe = ( kind == spell_kind::ultimate_aoe );
                raw *= axiom_arcanist_amp( source, is_aoe );
            }

            raw *= shadowflame_amp( source, target, type );
            return raw;
        }

        inline float apply_objective_reductions( float damage, game_object* source, game_object* target )
        {
            damage = baron_reduction(  damage, source, target );
            damage = dragon_reduction( damage, source, target );
            return damage;
        }

        inline float apply_crit( float raw, game_object* source, crit_state crit )
        {
            if ( crit != crit_state::crit ) return raw;
            return raw * source->get_crit_damage_multiplier();
        }

    }

    inline float spell_final(
        game_object* source,
        game_object* target,
        float        raw,
        damage_type  type,
        spell_kind   kind = spell_kind::normal )
    {
        if ( source == nullptr || target == nullptr ) return 0.f;

        const float amped     = detail::apply_spell_amps( raw, source, target, type, kind );
        const float mitigated = mitigate( amped, type, source, target );
        return detail::apply_objective_reductions( mitigated, source, target );
    }

    inline float auto_attack_final(
        game_object* source,
        game_object* target,
        float        raw,
        crit_state   crit = crit_state::no_crit )
    {
        if ( source == nullptr || target == nullptr ) return 0.f;

        raw  = detail::apply_crit( raw, source, crit );
        raw *= cut_down_amp(      source, target );
        raw *= coup_de_grace_amp( source, target );
        raw *= last_stand_amp(    source, target );

        const float mitigated = mitigate_physical( raw, source, target );
        return detail::apply_objective_reductions( mitigated, source, target );
    }

    inline bool will_kill( game_object* source, game_object* target, float final_damage )
    {
        if ( source == nullptr || target == nullptr ) return false;

        const float hp_plus_shield = target->get_hp() + target->get_all_shield();
        if ( final_damage >= hp_plus_shield ) return true;

        if ( elder_would_execute(     final_damage, source, target ) ) return true;
        if ( collector_would_execute( final_damage, source, target ) ) return true;

        return false;
    }

}
