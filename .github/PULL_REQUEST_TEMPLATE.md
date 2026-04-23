## Summary

One sentence: what this PR does and why.

## Type

- [ ] feat
- [ ] fix
- [ ] patch (Riot value update — link patch notes below)
- [ ] docs
- [ ] refactor
- [ ] test
- [ ] chore

## Checklist

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

## Patch notes link

For `patch:` PRs only.
