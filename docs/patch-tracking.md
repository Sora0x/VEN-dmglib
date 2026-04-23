# Patch tracking

Riot tunes runes, items, and objectives every patch (~2 weeks). When values shift in-game, the constants in this library go out of sync with reality. This page is the single source of truth for what version of those values the library currently encodes.

## Last full sweep

- **Riot patch:** 26.08
- **Date:** 2026-04-22
- **Verified by:** maintainer (manual cross-check against [official Riot patch notes](https://www.leagueoflegends.com/en-us/news/tags/patch-notes/))

If a value below has drifted in a more recent patch, please open an issue using the [patch update template](../.github/ISSUE_TEMPLATE/patch_update.md).

## Runes — `runes.hpp`

| Constant | Current value | League rule |
|---|---|---|
| `cut_down::ID` | `8017u` | Rune ID |
| `cut_down::AMP` | `0.08f` (8%) | Damage amplification when active |
| `cut_down::HP_THRESHOLD` | `0.60f` | Target must have > 60% max HP for the amp to fire |
| `coup_de_grace::ID` | `8014u` | Rune ID |
| `coup_de_grace::AMP` | `0.08f` (8%) | Damage amplification when active |
| `coup_de_grace::HP_THRESHOLD` | `0.40f` | Target must have < 40% HP |
| `last_stand::ID` | `8299u` | Rune ID |
| `last_stand::AMP_MIN` | `0.05f` (5%) | Minimum amp at the entry threshold |
| `last_stand::AMP_MAX` | `0.11f` (11%) | Maximum amp at or below the max threshold |
| `last_stand::HP_START` | `0.60f` | Source HP ratio at which amp begins (`AMP_MIN`) |
| `last_stand::HP_MAX` | `0.30f` | Source HP ratio at which amp peaks (`AMP_MAX`) |
| `axiom_arcanist::ID` | `8473u` | Rune ID |
| `axiom_arcanist::AMP` | `0.12f` (12%) | Amp on single-target ult |
| `axiom_arcanist::AMP_AOE` | `0.08f` (8%) | Amp on AoE ult |

## Items — `items.hpp`

| Constant | Current value | League rule |
|---|---|---|
| `shadowflame::ITEM_ID` | `4645` | Item ID |
| `shadowflame::AMP` | `0.20f` (20%) | Magic + true damage amp on low-HP targets |
| `shadowflame::HP_THRESHOLD` | `0.40f` | Target must have < 40% max HP |
| `collector::ITEM_ID` | `6676` | Item ID |
| `collector::EXECUTE_THRESHOLD` | `0.05f` (5%) | Execute fires when post-mitigation damage leaves target in `(0%, 5%]` of max HP |

## Objectives — `objectives.hpp`

| Constant | Current value | League rule |
|---|---|---|
| `baron::DEBUFF_HASH` | `0xD7D32CF6u` | Hash of the buff Baron applies to attackers |
| `baron::DAMAGE_REDUCTION` | `0.50f` (50%) | Reduction to attacker's damage against Baron Nashor |
| `dragon::VENGEANCE_HASH` | `0xEDFDC9u` | Hash of the Dragon's Ancient Grudge buff |
| `dragon::GRUDGE_PER_STACK` | `0.15f` (15%) | Reduction per dragon already killed by source's team (clamped ≥ 0) |
| `elder::BUFF_HASH` | `0x6B7098B1u` | Hash of the Elder Dragon execute buff |
| `elder::EXECUTE_THRESHOLD` | `0.20f` (20%) | Execute fires when post-mitigation damage leaves target ≤ 20% max HP |

## Mitigation formula — `mitigation.hpp`

The resist multiplier is **canonical League math**, not a tunable parameter:

```
positive resist:  100 / (100 + R)
negative resist:  2 - 100 / (100 - R)
```

The `100` and `2` literals are formula intrinsics, not patch-trackable values. Riot has not changed this formula in the documented history of the game. If they ever do, this page will call it out explicitly.

## How to update this page

When you submit a `patch:` PR:

1. Edit only the rows that changed
2. Update the "Last full sweep" patch number if every row was re-verified, OR add a footnote on changed rows like *"verified 26.10 — was 8% in 26.08 and earlier"*
3. Bump the patch badge in the [README](../README.md)
4. Link the official patch notes URL in your PR description

## Confidence by row type

- **Buff hashes** — extracted from a one-time dump of Riot's buff registry. Drift if Riot renames internal buffs (rare; usually visible as "dragon reduction stopped applying" reports from users).
- **Item IDs** and **rune IDs** — stable across patches; Riot rarely renumbers existing entries.
- **AMP / threshold values** — most patch-volatile. Verify these every patch if you can.
