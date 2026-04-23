# Overview

`VEN-dmglib` answers two questions:

1. **If I cast this spell at this enemy, how much damage actually lands?**
2. **Would that damage kill them, accounting for shields, Elder execute, and Collector?**

Everything in the library exists to support those two answers.

## The two paths

There are two ways to use the library, depending on how much control you want.

### Simple path — `compute.hpp`

For 95% of cases, two function calls do the work:

```cpp
const float final = damage::spell_final(
    player, enemy, raw, damage::damage_type::magic
);

if ( damage::will_kill( player, enemy, final ) )
{
    cast_q();
}
```

`spell_final` runs the full pipeline:

1. **Apply rune amps** — Cut Down, Coup de Grace, Last Stand, Axiom Arcanist
2. **Apply item amps** — Shadowflame
3. **Mitigate by damage type** — armor + armor pen for physical, MR + magic pen for magic, passthrough for true
4. **Apply objective reductions** — Baron debuff, Dragon Ancient Grudge

`will_kill` checks HP + shield, then folds in execute predicates from Elder Dragon and Collector.

### Primitive path — direct calls

When you need fine-grained control (debugging, hybrid spells, custom ordering), call the primitives directly:

```cpp
float raw = q_base + 0.4f * player->get_ability_power();

raw *= damage::cut_down_amp(      player, enemy );
raw *= damage::coup_de_grace_amp( player, enemy );
raw *= damage::last_stand_amp(    player, enemy );
raw *= damage::shadowflame_amp(   player, enemy, damage::damage_type::magic );

const float mitigated = damage::mitigate_magic( raw, player, enemy );
```

This is also the only path for **hybrid damage** spells (physical + magic in one cast). Mitigate each part separately, then sum:

```cpp
const float phys_part  = damage::mitigate_physical( raw_phys,  player, enemy );
const float magic_part = damage::mitigate_magic(    raw_magic, player, enemy );
const float total      = phys_part + magic_part;
```

The library deliberately has **no** `damage_type::mixed`. Any single-number "mixed" formula approximates with min-resist or 50/50 split — both produce values 5-15% off the real game math. See [CONTRIBUTING.md](../CONTRIBUTING.md) for the full reasoning.

## Module map

| File | What it covers |
|---|---|
| `mitigation.hpp` | Resist / pen math + `mitigate_*` entry points |
| `runes.hpp` | 4 rune amps (`cut_down`, `coup_de_grace`, `last_stand`, `axiom_arcanist`) |
| `items.hpp` | Shadowflame amp + Collector execute predicate |
| `objectives.hpp` | Baron / Dragon damage reductions + Elder execute predicate |
| `compute.hpp` | Composite pipeline (`spell_final`, `auto_attack_final`, `will_kill`) |

## Three opinionated design decisions

**1. Header-only.** No `.cpp` to compile, no library to link. Drop the headers in, include them, use them. The cost is slightly larger compile time per translation unit that includes them — acceptable at this library's scale (~5 files, none over ~150 lines).

**2. Files grouped by origin, not by function shape.** Collector is an item effect → it lives in `items.hpp` next to Shadowflame, even though Collector is a `bool would_execute` predicate and Shadowflame is a `float` amp. Elder execute lives in `objectives.hpp` next to Baron and Dragon reductions, even though Elder is a predicate and the others are reductions. This trades "find all executes in one place" for "find all rules about monster X in one place" — which reads more naturally for champion code writing targeted checks.

**3. SDK quirks are surfaced, not hidden behind generic abstractions.** `game_object*` can't be `const` because the VEN SDK isn't `const`-correct. `has_item()` needs a non-null `int*` out-parameter. Both are documented as one-liner exceptions and centralized via small `detail::` helpers. There is no abstract `unit_query_interface` adapter layer — that would be over-engineering for a single-host library. See [`sdk-dependency.md`](sdk-dependency.md) for the full SDK boundary.

## Related

- [`patch-tracking.md`](patch-tracking.md) — current value of every constant + which Riot patch each was last verified against
- [`sdk-dependency.md`](sdk-dependency.md) — every VEN SDK piece the library touches
- [README](../README.md) — install instructions and quick example
- [CONTRIBUTING](../CONTRIBUTING.md) — full style contract for PRs
