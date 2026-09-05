# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this
repository.

## Issue tracking (Plane)

Project **GYC** (key `GYC`), workspace `ymir-bootstrap`. Use the plane MCP tools.

Work items used to live in Linear under a single `YMI` key shared by every repository, which
is why older PR titles and branch names here all read `YMI-`. Each repository now has its own
Plane project, and new work for this one goes under `GYC`.

## Pull request titles

PR titles must read `[GYC-XXX][kind] Log` — `GYC-XXX` is the Plane work item, and `[kind]` is
optional and defaults to a feature. Known kinds: `feat`/`feature`, `fix`, `perf`, `refactor`,
`doc(s)`, `test(s)`, `chore`/`ci`/`build`/`style`, `breaking`. This is not cosmetic: the release
notes are generated from these titles by `.github/scripts/changelog.sh`, one entry per merged PR
(the commits inside a PR are never listed), grouped by kind. **A PR whose title does not follow
the format — no issue key, or a kind outside that list — is left out of the release notes
entirely**; the skip is logged on stderr by the release job, but the change goes unannounced.

The leading tag is also the only thing tying a PR to Plane. `.github/workflows/plane-sync.yml`
takes the work item from it and nowhere else — a body merely mentioning another item cannot
attach the PR to it — then attaches the PR as a link, comments on open, close and merge, and
moves the item to In Progress when the PR opens and to Done when it merges. A title without a
valid tag is skipped in silence, so it costs both the release note and the tracker update.

The branch currently checked out is generally named `GYC-<issue_number>-<short-description>`
(e.g. `GYC-8-plane-ci`) — the `GYC-<issue_number>` part is the Plane work item key, so it
can be used to look up the item this branch's work is tracked against.
