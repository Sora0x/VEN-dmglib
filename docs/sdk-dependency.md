# SDK dependency

`VEN-dmglib` is **not** a standalone library. It is built on top of the [VEN platform's SDK](https://docs.ven.pub/) and uses several primitives from it. This page lists every SDK piece the library touches and why.

## SDK headers used

The library `#include`s only the umbrella SDK header:

```cpp
#include <sdk.hpp>
```

That single include pulls in every SDK type and global the library needs.

## Types referenced

| Type | Used for | Files |
|---|---|---|
| `game_object*` | Source champion and target unit | every `.hpp` |
| `buff_instance*` | Buff queries (rune/item state, dragon stacks, baron debuff, elder buff) | `objectives.hpp` |
| `g_sdk` (global) | Logging only (`g_sdk->log_console` for unexpected failures) | not currently used in `include/`; reserved |

The library defines its own `damage_type` enum in `mitigation.hpp` rather than reusing the SDK's `dmg_sdk::damage_type` — the two have different semantics (the lib's enum has `true_damage`; the SDK's may not).

## `game_object*` methods called

These are the SDK methods the library actually invokes. If anyone ever wants to port the library to a different host, this is the adapter contract.

### Stats and identity

| Method | Where |
|---|---|
| `get_armor()` | `mitigation.hpp` |
| `get_bonus_armor()` | `mitigation.hpp` |
| `get_magic_resist()` | `mitigation.hpp` |
| `get_bonus_magic_resist()` | `mitigation.hpp` |
| `get_physical_lethality()` | `mitigation.hpp` |
| `get_percent_armor_penetration()` | `mitigation.hpp` |
| `get_percent_bonus_armor_penetration()` | `mitigation.hpp` |
| `get_flat_magic_penetration()` | `mitigation.hpp` |
| `get_percent_magic_penetration()` | `mitigation.hpp` |
| `get_percent_bonus_magic_penetration()` | `mitigation.hpp` |
| `get_hp()` | runes / items / objectives (HP-threshold checks, execute checks) |
| `get_max_hp()` | runes / items / objectives (HP-ratio computation) |
| `get_all_shield()` | items / objectives (execute checks fold shields into "killable") |
| `get_crit_damage_multiplier()` | `compute.hpp` (auto-attack crit pipeline) |
| `get_char_name()` | `objectives.hpp` (`is_baron_nashor`, `is_dragon`) |
| `is_hero()` | `runes.hpp` (rune amps gate on hero targets) |
| `is_epic_monster()` | `objectives.hpp` (`is_dragon`) |

### Inventory and buffs

| Method | Where |
|---|---|
| `has_item( int id, int* out_slot )` | `items.hpp` (via `detail::source_has_item`) |
| `has_rune( uint32_t id )` | `runes.hpp` |
| `get_buff_by_hash( uint32_t hash )` | `objectives.hpp` |
| `get_buffs()` | `objectives.hpp` (`count_dragons_killed`) |

## `buff_instance*` methods called

| Method | Where |
|---|---|
| `is_active()` | `objectives.hpp` (gate buff queries on active state) |
| `get_name()` | `objectives.hpp` (string match on dragon-kill buff family) |
| `get_stacks()` / `get_count()` | `objectives.hpp` (`count_dragons_killed`) |

## SDK-driven design choices

A few quirks of the VEN SDK directly shaped the library's API.

### `game_object*` cannot be `const`

The SDK's `get_*` methods on `game_object` are not declared `const`. As a result, every function in this library that queries a `game_object*` takes it as `game_object*`, not `const game_object*`. This is the only accepted exception to the "const everywhere" rule — see [CONTRIBUTING.md](../CONTRIBUTING.md).

### `has_item` requires a non-null `int*` out-parameter

The SDK signature is:

```cpp
virtual bool has_item( int item_id, int* found_slot ) = 0;
```

Even when the caller doesn't care about the slot, `found_slot` cannot be `nullptr`. The library hides this with `detail::source_has_item( source, item_id )` so the dummy `int` lives in exactly one place.

### Buff hashes vs buff names

Some objectives logic queries buffs by hash (`get_buff_by_hash( elder::BUFF_HASH )`); some by name string fragment (`buff->get_name().find( "SRX_DragonBuff" )`). Hashes are faster and more robust to Riot's internal changes. Name matching is used only when there is no public hash for the variant family — currently only inside `count_dragons_killed`, where the dragon-kill buff has multiple variants per dragon type and we just check the name prefix.

If a Riot patch renames internal buffs, the name-based queries drift first. See [`patch-tracking.md`](patch-tracking.md) for confidence notes.

## Porting to a different host

Every formula in the library is plain math; only the boundary types are SDK-specific. Porting would mean providing:

1. A `game_object`-shaped type implementing the methods listed above
2. A `buff_instance` type implementing `is_active`, `get_name`, `get_stacks`, `get_count`
3. A thin compatibility `<sdk.hpp>` that pulls those types in

That work isn't trivial but is feasible. A `docs/porting.md` may eventually capture the contract more formally if a port is actually attempted.

For now: the library is **VEN platform only**.
