# VEN - Community Damage Library
Header-only C++20 library that implements League of Legends damage math. Built for the [VEN Platform](https://docs.ven.pub/).

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL_2.0-brightgreen.svg)](https://www.mozilla.org/en-US/MPL/2.0/)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Patch](https://img.shields.io/badge/Riot_patch-26.08-orange.svg)](docs/patch-tracking.md)

## What it does
You give it a raw damage number plus context (source champion, target, damage type). It hands back the final damage that actually lands by calculating armor and MR mitigation, penetration, rune amps, item amps, objective damage reductions. There's also a `will_kill` predicate that folds HP, shields, and execute checks (Elder, Collector) into one boolean.

## Who it's for
VEN platform plugin developers. If you're writing a champion plugin and you need to predict combo damage to decide whether to all-in, or by casting an execute spell (Twitch E, Kalista E) this is what you reach for.

It also works as a reference if you're learning C++ for VEN plugin development. The code is intentionally readable, and the formulas match what the game actually applies.

## Requirements
- **VEN SDK headers.** The library uses `game_object*`, `buff_instance*`, and `g_sdk` from the VEN platform. You can't build without the SDK. Get it from [docs.ven.pub](https://docs.ven.pub/).
- **C++20.** MSVC v143 is recommended, it's the toolchain I tested against. GCC and Clang in C++20 mode should work but no CI yet.
- Header-only. No `.cpp` to compile, no library to link. Drop the headers in, include them, use them.

## Quick example
```cpp
#include "ven_dmglib/compute.hpp"

using namespace venplus::damage;

// Twitch E magic damage scales with passive stacks
auto* poison = enemy->get_buff_by_hash( DEADLY_VENOM );
const int stacks = poison ? poison->get_count() : 0;

// get twitch damage
const float raw = ( 60.f + 55.f * stacks )
                + 0.4f * player->get_ability_power();

// calculate
const float final_damage = spell_final( player, enemy, raw, damage_type::magic );

// execution
if ( will_kill( player, enemy, final_damage ) )
{
    cast_e();
}
```

`spell_final` runs the full pipeline: rune amps -> Shadowflame amp -> mitigation by damage type -> Baron / Dragon reductions (if they are the target). `will_kill` checks HP + shield + Elder execute + Collector execute.

The direct path when you want fine control:

```cpp
#include "ven_dmglib/runes.hpp"
#include "ven_dmglib/items.hpp"
#include "ven_dmglib/mitigation.hpp"

float raw = q_base + 0.4f * player->get_ability_power();

raw *= cut_down_amp     ( player, enemy );
raw *= coup_de_grace_amp( player, enemy );
raw *= last_stand_amp   ( player, enemy );
raw *= shadowflame_amp  ( player, enemy, damage_type::magic );

const float final = mitigate_magic( raw, player, enemy );
```

## What's covered for now
| Module | Covers |
|---|---|
| `mitigation.hpp` | Resist/pen math, mitigate by damage type (physical/magic/true) |
| `runes.hpp` | Cut Down, Coup de Grace, Last Stand, Axiom Arcanist |
| `items.hpp` | Shadowflame (amp), Collector (execute) |
| `objectives.hpp` | Baron damage reduction (against baron), Dragon damage reduction (against dragons), Elder Dragon execute |
| `compute.hpp` | Composite pipeline (`spell_final`, `auto_attack_final`, `will_kill`) |

## Not covered yet
- Summoner spells (Ignite, Exhaust, Smite) — PRs welcome
- On-hit items (Runaan's, Rageblade) — PRs welcome
- Spellblade procs (Sheen, Trinity, Lichbane) — PRs welcome
- Champion-specific buff that modifies general damages (Nami E ally buff, Janna E, etc.) — PRs welcome
- Hybrid `mixed` damage type — by design. Mixed in League is two separate damage events; the caller splits into two `mitigate_*` calls and sums
- Health Prediction — PRs welcome

## Install
### As a vendored copy
Copy `include/ven_dmglib/` into your project. Add the parent path to your includes. Done.

### As a git submodule (recommended)

```bash
cd <your-plugin-project>
git submodule add https://github.com/Sora0x/VEN-dmglib.git external/VEN-dmglib
```

In your `vcxproj`, add the include path:

```xml
<AdditionalIncludeDirectories>external/VEN-dmglib/include;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
```

Then `#include "ven_dmglib/compute.hpp"` and use.

## Documentation

- [Overview](docs/overview.md) — architecture, the two paths, design rationale
- [mitigation.md](docs/mitigation.md) — armor/MR formulas, penetration model
- [runes.md](docs/runes.md) — the 4 covered runes, conditions, mutual exclusivity
- [items.md](docs/items.md) — Shadowflame amp, Collector execute window
- [objectives.md](docs/objectives.md) — Baron / Dragon / Elder rules and quirks
- [compute.md](docs/compute.md) — composite pipeline + `will_kill`
- [sdk-dependency.md](docs/sdk-dependency.md) — what VEN SDK pieces are used and why
- [patch-tracking.md](docs/patch-tracking.md) — which Riot patch each constant matches

## Contributing

PRs are welcome. The code style is strict — see [CONTRIBUTING.md](CONTRIBUTING.md) for the full rules.

The five that matter most:

1. No comments. If the code needs one, rewrite the code until you can read it without explanation (then you are sure that everyone can read it). If you want to help a beginner, create an implementation example and there you can comment.

2. No magic numbers, except formula intrinsics like the `100` in `100/(100+R)`.
3. One function, one responsibility. Cap: 25 lines per body, cuts insane nesting at root.
4. `const` everywhere it can go.
5. Names a non-coder can read.

If a PR breaks any of those, the review will ask for a rewrite. Not pickiness, just the contract that keeps the library readable as more contributors arrive.

## Patch tracking

Riot tunes runes, items, and objectives every patch (~2 weeks). When values shift in-game, the constants here drift out of sync. The current state lives in [docs/patch-tracking.md](docs/patch-tracking.md).

If you spot a Riot change that affects something here, open an issue with the [patch update template](.github/ISSUE_TEMPLATE/patch_update.md). PRs that change constants must link the official patch notes.

## License

[Mozilla Public License 2.0](LICENSE).

In plain English: you can use this library in any project, including closed-source / commercial plugins. If you modify a file in this library and ship the modified version, you must publish your modifications to those specific files. Your own plugin code stays under whatever license you want — only the modified `damage/*.hpp` files are subject to MPL.

## Acknowledgements

Built originally for VEN+ (closed-source plugin) and extracted into its own library so the rest of the VEN platform can use it.

Damage formulas were verified against the [League of Legends Wiki](https://wiki.leagueoflegends.com/) and tested in real games.

The "code is the documentation" approach was inspired by years of debugging plugins where the code lied and the comments were stale.
