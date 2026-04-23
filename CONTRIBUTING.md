# Contributing to VEN-dmglib

Thanks for considering a contribution. This library exists so VEN platform plugin developers don't reinvent damage math per champion. Every PR makes that math more correct, more readable, or more covered.

## Before you contribute

- You have access to the VEN SDK headers ([docs.ven.pub](https://docs.ven.pub/))
- You have a C++20 compiler (MSVC v143 / Visual Studio 2026 BuildTools 18+ is the reference)
- You've read this document. The style contract is strict — PRs that ignore it get rewrite requests.

## Development setup

```bash
git clone https://github.com/Sora0x/VEN-dmglib.git
cd VEN-dmglib
```

Point your build at the VEN SDK headers. Everything lives in `include/ven_dmglib/` and is header-only.

## Philosophy

A reader who has never coded League scripts should understand the intent from the names alone. Comments lie, code doesn't. Every rule below serves that north star.

## The five non-negotiables

Break any of these and the PR gets a rewrite request, not a merge.

### 1. No comments in `include/`

If the code needs a comment, rewrite it. 3 one-line exceptions:

- `// TODO:` on data not yet implemented that you plan, or wants another contributor to implement
- License header at file top
- Language escape hatch (e.g., a getter forced non-`const` by an SDK quirk) — justified in the PR

Tutorial files under `examples/` can have explanatory comments — those exist to teach.

### 2. No magic numbers

Every numeric literal outside `0`, `1`, `-1`, `0.f`, `1.f` is a named `constexpr`. **One exception — formula intrinsics.** The `100` in `100/(100+R)` and the `2` in `2 - 100/(100-R)` are part of League's canonical resist formula, not tunable parameters. Naming them adds verbosity without clarity; the function name (`resist_multiplier`) already carries the intent.

### 3. One function, 25-line cap

Body ≤ 25 lines (excluding blanks and braces). If the name contains "and", split. Nesting depth ≤ 2 levels of `{ }` inside a body.

### 4. `const` everywhere it can go

Borrowed params not mutated -> `const T&` or `const T*`. Locals assigned once -> `const`. Non-mutating methods -> `const`.

**Exception: `game_object*` cannot be `const`.** The VEN SDK's getters aren't marked `const`, so functions that query a `game_object*` take it non-`const`. Don't `const_cast` around it; just drop the `const` on the parameter.

### 5. Names a non-coder can read

`calculate_damage` not `calc_dmg`. `target_hp_ratio` not `tgt_hp_pct`. `effective_resist` not `eff_res`. Acronyms are allowed only when they're the actual name of the thing (`hp`, `mp`, `id`, `sdk`, `cd` for cooldown), lowercase.

## Style contract

### Naming shapes used in this library

| Shape | Used for | Example |
|---|---|---|
| `is_x()` | Bool predicate | `is_dragon`, `is_baron_nashor` |
| `count_x()` | Counting accessor | `count_dragons_killed` |
| `<rune>_amp()` / `<item>_amp()` | Returns damage multiplier (`1.0` = no-op) | `cut_down_amp`, `shadowflame_amp` |
| `<mechanic>_reduction()` | Returns adjusted damage | `baron_reduction`, `dragon_reduction` |
| `<mechanic>_would_execute()` | Predicts execute | `elder_would_execute`, `collector_would_execute` |
| `mitigate_<type>()` | Applies target's defenses | `mitigate_physical`, `mitigate_magic` |
| `<action>_final()` | Full pipeline composition | `spell_final`, `auto_attack_final` |
| `struct foo` | Data only, zero methods | `struct cut_down`, `struct penetration` |
| `struct::FIELD` | Compile-time constant | `cut_down::AMP`, `shadowflame::HP_THRESHOLD` |

Files, namespaces, methods, locals: `snake_case`. Member fields: `snake_case_` with trailing underscore.

### Formatting

```cpp
// Allman braces, `if ( x )` spacing, `T*` and `T&` glued to the type.
float mitigate_magic( float raw, game_object* source, game_object* target )
{
    if ( source == nullptr || target == nullptr ) return raw;
    // ...
}
```

4-space indent, 100-column soft limit, 120 hard. `switch` cases braced, no silent fall-through. No aligned `=` across declarations (creates noisy diffs).

### Constants placement

Each mechanic gets its own `struct` of `static constexpr` values, colocated with its function in the same header:

```cpp
struct cut_down
{
    static constexpr uint32_t ID           = 8017u;
    static constexpr float    AMP          = 0.08f;
    static constexpr float    HP_THRESHOLD = 0.60f;
};
```

Access reads as a table lookup: `cut_down::AMP`. No header-level globals across files.

### Types

`struct` = data, zero methods, all public, brace-init defaults on every member. Queries about a data struct are **free functions in the same namespace**, never methods on the struct.

`class` reserved for behavior with invariants — rare here.

### No exceptions, no asserts, no new globals

The library runs inside a VEN host DLL. Use `bool` returns or `std::optional< T >` for fallible ops. `assert( ... )` is compiled out in release — use early returns + `g_sdk->log_console( "[!] ..." )` for unexpected failures. The only allowed global is `g_sdk` from `<sdk.hpp>`.

## Hybrid damage convention

**There is no `damage_type::mixed`.** Mixed damage in League is two separate damage events. The caller splits:

```cpp
const float phys_part  = mitigate_physical( raw_phys,  source, target );
const float magic_part = mitigate_magic(    raw_magic, source, target );
const float total      = phys_part + magic_part;
```

PRs adding a `mixed` type will be rejected. Any single-number `mixed` formula (min-resist, 50/50 split, configurable ratio) is 5-15% off the real game math.

## Adding a new mechanic

Read the existing implementations and mirror them exactly.

### New rune — mirror `cut_down` in `runes.hpp`

```cpp
struct cut_down
{
    static constexpr uint32_t ID           = 8017u;
    static constexpr float    AMP          = 0.08f;
    static constexpr float    HP_THRESHOLD = 0.60f;
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
```

Constants struct + free function named `<rune>_amp`. Null-guard, rune check, condition, return `1.f` (no-op) or `1.f + AMP`.

### New item — mirror `shadowflame` in `items.hpp`

```cpp
struct shadowflame
{
    static constexpr int   ITEM_ID      = 4645;
    static constexpr float AMP          = 0.20f;
    static constexpr float HP_THRESHOLD = 0.40f;
};

inline float shadowflame_amp( game_object* source, game_object* target, damage_type type )
{
    if ( target == nullptr ) return 1.f;
    if ( type == damage_type::physical ) return 1.f;
    if ( !detail::source_has_item( source, shadowflame::ITEM_ID ) ) return 1.f;

    const float target_hp_ratio = target->get_hp() / target->get_max_hp();
    if ( target_hp_ratio >= shadowflame::HP_THRESHOLD ) return 1.f;

    return 1.f + shadowflame::AMP;
}
```

Use `detail::source_has_item(source, ID)` — it null-guards source and hides the SDK's mandatory out-parameter for `has_item()`. Don't reinvent the dummy `int slot{ 0 }` boilerplate at every call site.

### After the function exists

1. Wire into `compute.hpp` if `spell_final` / `auto_attack_final` should apply the mechanic automatically
2. Update `docs/patch-tracking.md` with the new constants and current Riot patch
3. Update `README.md` "What's covered" if it fills a row
4. Add a unit test under `tests/test_<file>.cpp` with at least one true case and one false case

## Adding a value update (Riot tuned a constant)

1. Open an issue using the [patch update template](.github/ISSUE_TEMPLATE/patch_update.md), linking the official patch notes
2. Open a PR that updates only the affected constants
3. Update `docs/patch-tracking.md` with the new "last verified" patch
4. Bump the patch badge in `README.md`
5. Commit: `patch(<area>): <what changed>` — e.g., `patch(runes): cut_down amp 8% -> 9% in 26.10`

## PR checklist

- [ ] No new comments in `include/` outside the four exceptions
- [ ] Every literal is named, OR a documented formula intrinsic
- [ ] No function body exceeds 25 lines
- [ ] No function name contains "and"
- [ ] `const` on every non-mutating parameter and local (`game_object*` exempted)
- [ ] `nullptr` check on every borrowed pointer at the boundary
- [ ] Allman braces, `if ( x )` spacing, `T*`/`T&` glued
- [ ] Tests added for new public functions
- [ ] `docs/patch-tracking.md` updated if constants changed
- [ ] Commit messages follow Conventional Commits

## Commit message format

```
<type>(<scope>): <short summary>
```

**Types:**
- `feat:` — new public function or mechanic
- `fix:` — bug correction in existing logic
- `patch:` — Riot value update (include patch number in the message)
- `docs:` — documentation only
- `refactor:` — internal restructure, no behavior change
- `test:` — test-only changes
- `ci:` — CI config changes
- `chore:` — tooling, build files

**Scope** (encouraged): `mitigation`, `runes`, `items`, `objectives`, `compute`, `docs`, `tests`.

**Examples:**

```
feat(items): add Rageblade on-hit damage
fix(items): collector window must exclude hp_after <= 0
patch(runes): cut_down amp 8% -> 9% in 26.10
docs(overview): clarify hybrid damage convention
test(objectives): add elder execute boundary cases
```

Summary imperative, lowercase, no trailing period, ≤ 72 chars.

## Code review

Single maintainer: **Sora** (`nk_sora` on Discord).

- Reviews are best effort. Patch updates get prioritized — drifted values silently hurt consumers.
- One maintainer approval required to merge.
- We squash on merge.

Want to co-maintain? Ping `nk_sora` on Discord.

## Code of Conduct

[Contributor Covenant 2.1](CODE_OF_CONDUCT.md). Be kind, assume good intent, no harassment.

## Questions

GitHub Discussions (preferred) or an issue with the `question` label. Private: `nk_sora` on Discord.
