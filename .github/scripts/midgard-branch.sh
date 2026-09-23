#!/usr/bin/env bash
# Resolves the midgard a gyc build bundles: the head of a github.com/GNU-Ymir/yruntime branch
# (YMIR_VERSION's MIDGARD_BRANCH), never a release tag. A yruntime release is built with the gyc
# that bundles it, so it can only be tagged once that gyc is out - requiring the tag up front
# would be a cycle neither side could break. release.yml marks the commit with a temporary tag
# while it builds, then hands the branch over to yruntime's own release, which tags it for real
# and merges its pull request.
#
# usage: midgard-branch.sh --branch=<name> --expect-gyc=<GYC_VERSION> [--mode=release|preview]
#
# Prints, one `key=value` per line on stdout (append it to $GITHUB_OUTPUT or $GITHUB_ENV):
#   midgard_version       the top-level `version` of the branch's gyllir.toml
#   midgard_sha           the branch head, pinned so every job of a run builds the same commit
#                         (and so BuildKit busts its layers when the branch moves, which a
#                         constant name would not)
#   midgard_version_date  its commit date, YYYYMMDD, reported by `gyc --version`
#   midgard_temp_tag      the temporary tag release.yml marks that commit with while it builds
#   midgard_pr            the number of the branch's open pull request, empty for the default
#                         branch (release mode only)
#
# What makes the branch releasable, all checked here:
#   - its version is not tagged on yruntime yet: the tag is what yruntime's release creates once
#     this gyc is out, so an existing one means the version was not bumped;
#   - its YMIR_VERSION builds with the gyc being released (--expect-gyc): yruntime's release
#     compiles midgard with that gyc, and nothing else tells it the new one exists;
#   - unless it is the default branch, it has an open pull request, which yruntime's release
#     merges once it has tagged the commit.
# `--mode=release` (the default) fails on any of them; `--mode=preview` only warns and skips the
# pull request lookup - a preview has no release of its own to protect.
set -euo pipefail

BRANCH=""
EXPECT_GYC=""
MODE=release
for arg in "$@"; do
  case "$arg" in
    --branch=*) BRANCH="${arg#--branch=}" ;;
    --expect-gyc=*) EXPECT_GYC="${arg#--expect-gyc=}" ;;
    --mode=release|--mode=preview) MODE="${arg#--mode=}" ;;
    *) echo "usage: $0 --branch=<name> --expect-gyc=<version> [--mode=release|preview]" >&2; exit 2 ;;
  esac
done
if [ -z "$BRANCH" ] || [ -z "$EXPECT_GYC" ]; then
  echo "::error::--branch and --expect-gyc are required - see YMIR_VERSION's MIDGARD_BRANCH and GYC_VERSION" >&2
  exit 2
fi

SLUG="GNU-Ymir/yruntime"
REPO="https://github.com/${SLUG}.git"

problems=0
problem() {
  if [ "$MODE" = release ]; then echo "::error::$*" >&2; problems=1; else echo "::warning::$*" >&2; fi
}

CLONE="$(mktemp -d)"
trap 'rm -rf "$CLONE"' EXIT

# Bare, blobless, one commit deep: the few small files read below are fetched on demand.
if ! git clone --quiet --bare --filter=blob:none --depth 1 --branch "$BRANCH" "$REPO" "$CLONE" 2>/dev/null; then
  echo "::error::${SLUG} has no branch ${BRANCH} (YMIR_VERSION's MIDGARD_BRANCH)" >&2
  exit 1
fi

SHA="$(git -C "$CLONE" rev-parse HEAD)"
SHORT="${SHA:0:7}"
DATE="$(git -C "$CLONE" log -1 --format=%cd --date=format:%Y%m%d HEAD)"

# Only the `version` above the first [table] header is the package's own.
VERSION="$(git -C "$CLONE" show HEAD:gyllir.toml | sed -nE '/^\[/q; s/^version *= *"([^"]+)".*/\1/p')"
if ! printf '%s\n' "$VERSION" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+$'; then
  echo "::error::${SLUG}@${BRANCH} (${SHORT})'s gyllir.toml has no <major>.<minor>.<patch> top-level version (got '${VERSION}')" >&2
  exit 1
fi

if git ls-remote --exit-code --tags "$REPO" "refs/tags/${VERSION}" >/dev/null 2>&1; then
  problem "${SLUG} already has a tag ${VERSION}, the version ${BRANCH} (${SHORT}) declares - bump yruntime's version before building a gyc that bundles it."
fi

THEIR_GYC="$(git -C "$CLONE" show HEAD:YMIR_VERSION 2>/dev/null | sed -nE 's/^YMIR_BOOTSTRAP_VERSION=(.*)/\1/p' | tail -n1)"
if [ "$THEIR_GYC" != "$EXPECT_GYC" ]; then
  problem "${SLUG}@${BRANCH} (${SHORT})'s YMIR_VERSION builds with gyc '${THEIR_GYC}', not ${EXPECT_GYC} - its release, run once this gyc is out, would compile midgard with the wrong compiler."
fi

PR=""
DEFAULT="$(git ls-remote --symref "$REPO" HEAD | sed -nE 's|^ref: refs/heads/([^[:space:]]+)[[:space:]]+HEAD$|\1|p')"
if [ "$MODE" = release ] && [ "$BRANCH" != "$DEFAULT" ]; then
  PR="$(gh pr list --repo "$SLUG" --head "$BRANCH" --state open --json number --jq '.[0].number // empty')"
  if [ -z "$PR" ]; then
    problem "${SLUG} has no open pull request from ${BRANCH} - yruntime's release merges it once it has tagged ${VERSION}."
  fi
fi

[ "$problems" = 0 ] || exit 1

echo "midgard_version=${VERSION}"
echo "midgard_sha=${SHA}"
echo "midgard_version_date=${DATE}"
# Not <version>-temp: yruntime's `tags` ruleset forbids deleting anything matching *.*.*, and in a
# ruleset pattern `*` does not cross a `/` - this name is the one it lets the workflow remove.
echo "midgard_temp_tag=temp/${VERSION}"
echo "midgard_pr=${PR}"
