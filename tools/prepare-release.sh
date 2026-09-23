#!/usr/bin/env bash
# Opens the pull requests preparing the next gyc release, one per repository:
#   - yruntime: a branch declaring the midgard that gyc bundles, built with that gyc;
#   - gymir:    YMIR_VERSION releasing that gyc and bundling the yruntime branch above;
#   - CD_suite: the bootstrap chain stages of the gyc releases it does not list yet.
# Bootstrap is the value of truth and is bumped by hand beforehand: what is released and what
# compiles it are read from its default branch, then every value is asked with that as default.
# Everything happens in fresh clones under /tmp, removed on exit, so no local checkout is read
# or written. --dry-run prints the commits and pushes nothing.
#
# usage: tools/prepare-release.sh [--dry-run]
set -euo pipefail

DRY_RUN=0
case "${1:-}" in
  "") ;;
  --dry-run) DRY_RUN=1 ;;
  *) echo "usage: $0 [--dry-run]" >&2; exit 2 ;;
esac

TOOLS="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORK="$(mktemp -d /tmp/prepare-release.XXXXXX)"
trap 'rm -rf "$WORK"' EXIT
GYMIR="$WORK/gymir"
BOOTSTRAP="$WORK/bootstrap"
YRUNTIME="$WORK/yruntime"
CD_SUITE="$WORK/CD_suite"
GYLLIR_REPO="https://github.com/GNU-Ymir/Gyllir.git"
SEMVER='^[0-9]+\.[0-9]+\.[0-9]+$'

die() { echo "error: $*" >&2; exit 1; }
warn() { echo "warning: $*" >&2; }

default_branch() { git -C "$1" symbolic-ref --short refs/remotes/origin/HEAD 2>/dev/null | sed 's|^origin/||' || echo master; }
slug() { git -C "$1" remote get-url origin | sed -E 's#^.*github\.com[:/]##; s#\.git$##'; }
at_default() { git -C "$1" show "origin/$(default_branch "$1"):$2"; }
pkg_version() { sed -nE '/^\[/q; s/^version *= *"([^"]+)".*/\1/p'; }
std_version() { sed -nE '/^\[std\]/,/^\[/ s/^version *= *"([^"]+)".*/\1/p' | head -n1; }
setting() { sed -nE "s/^$1=(.*)/\1/p" | tail -n1; }
releases() { git -C "$1" tag -l | grep -E "$SEMVER" | sort -V; }
is_released() { releases "$1" | grep -xF "$2" >/dev/null; }
next_minor() { local a b; IFS=. read -r a b _ <<< "$1"; echo "$a.$((b + 1)).0"; }
has_branch() { git -C "$1" ls-remote --exit-code --heads origin "$2" >/dev/null 2>&1; }
human_join() { # a | a and b | a, b and c
  local n=$#
  if [ "$n" -le 1 ]; then echo "$*"; else echo "$(IFS=,; echo "${*:1:n-1}" | sed 's/,/, /g') and ${!n}"; fi
}

# ask <var> <question> <default> [<pattern>]: reads the terminal until the answer matches.
ask() {
  local answer
  while :; do
    read -r -p "$2 [$3]: " answer </dev/tty
    answer="${answer:-$3}"
    if [ -z "${4:-}" ] || [[ "$answer" =~ $4 ]]; then printf -v "$1" '%s' "$answer"; return; fi
    echo "  expected $4" >&2
  done
}

# set_setting <YMIR_VERSION file> <key> <value>
set_setting() {
  grep -q "^$2=" "$1" || die "$1 has no $2"
  sed -i -E "s|^$2=.*|$2=$3|" "$1"
}

command -v gh >/dev/null || die "gh is required"
[ "$DRY_RUN" = 1 ] || gh auth status >/dev/null 2>&1 || die "gh is not authenticated"

# Blobless: every commit, tree and tag, and only the file contents actually read.
echo "Cloning gymir, bootstrap, yruntime and CD_suite in $WORK..."
for pair in "gymir|$GYMIR" "bootstrap|$BOOTSTRAP" "yruntime|$YRUNTIME" "CD_suite|$CD_SUITE"; do
  git clone --quiet --filter=blob:none "git@github.com:GNU-Ymir/${pair%%|*}.git" "${pair#*|}"
done

# ---------------------------------------------------------------------------------------------
# The versions
# ---------------------------------------------------------------------------------------------

B_TOML="$(at_default "$BOOTSTRAP" gyllir.toml)"
B_VERSION="$(at_default "$BOOTSTRAP" YMIR_VERSION)"

gyc_default="$(pkg_version <<< "$B_TOML")"
ask GYC "gyc to release (GYC_VERSION)" "$gyc_default" "$SEMVER"
! is_released "$GYMIR" "$GYC" || die "gymir already has a release $GYC - bump bootstrap's gyllir.toml first"
[ "$GYC" = "$gyc_default" ] || warn "bootstrap declares $gyc_default, but a release tags the same version on both repositories"

IFS=. read -r MAJOR MINOR _ <<< "$GYC"
compiler_default="$(setting YMIR_BOOTSTRAP_VERSION <<< "$B_VERSION")"
if [ "$MINOR" -gt 0 ]; then
  expected="$(releases "$GYMIR" | grep -E "^$MAJOR\.$((MINOR - 1))\." | tail -n1 || true)"
  if [ -n "$expected" ] && [ "$expected" != "$compiler_default" ]; then
    warn "bootstrap is compiled by gyc $compiler_default, but $GYC compiles from $expected, the last $MAJOR.$((MINOR - 1)) release - bump bootstrap's YMIR_VERSION first"
  fi
fi
ask COMPILER "gyc compiling it (YMIR_BOOTSTRAP_VERSION)" "$compiler_default" "$SEMVER"
is_released "$GYMIR" "$COMPILER" || die "gymir has no release $COMPILER"

compiler_midgard_default="$(setting MIDGARD_VERSION <<< "$B_VERSION")"
std="$(std_version <<< "$B_TOML")"
[ "$std" = "$compiler_midgard_default" ] || warn "bootstrap's YMIR_VERSION says midgard $compiler_midgard_default, its gyllir.toml [std] says $std"
ask COMPILER_MIDGARD "midgard it is compiled against (YMIR_BOOTSTRAP_MIDGARD_VERSION)" "$compiler_midgard_default" "$SEMVER"
is_released "$YRUNTIME" "$COMPILER_MIDGARD" || die "yruntime has no release $COMPILER_MIDGARD"

ask GCC "gcc (GCC_VERSION)" "$(setting GCC_VERSION <<< "$B_VERSION")" "$SEMVER"
ask GYLLIR "gyllir (GYLLIR_VERSION)" "$(setting GYLLIR_VERSION <<< "$B_VERSION")" "$SEMVER"

current_midgard="$(at_default "$YRUNTIME" gyllir.toml | pkg_version)"
midgard_default="$current_midgard"
! is_released "$YRUNTIME" "$current_midgard" || midgard_default="$(next_minor "$current_midgard")"
ask MIDGARD "midgard gyc $GYC bundles" "$midgard_default" "$SEMVER"
! is_released "$YRUNTIME" "$MIDGARD" || die "yruntime already has a release $MIDGARD"

ask GYC_KEY "gymir work item" "" '^GYC-[0-9]+$'
ask MID_KEY "yruntime work item" "" '^MID-[0-9]+$'
ask CD_KEY "CD_suite work item (empty for none)" "" '^([A-Z]+-[0-9]+)?$'

GYMIR_BRANCH="$GYC_KEY-prepare-$GYC"
MIDGARD_BRANCH="$MID_KEY-compile-from-$MAJOR.$MINOR"
for pair in "$GYMIR|$GYMIR_BRANCH" "$YRUNTIME|$MIDGARD_BRANCH"; do
  ! has_branch "${pair%%|*}" "${pair#*|}" || die "$(slug "${pair%%|*}") already has a branch ${pair#*|}"
done

# ---------------------------------------------------------------------------------------------
# The branches
# ---------------------------------------------------------------------------------------------

commit() { git -C "$1" commit --quiet -am "$2"; }
has_commits() { [ -n "$(git -C "$1" rev-list "origin/$(default_branch "$1")..HEAD")" ]; }

set_setting "$YRUNTIME/YMIR_VERSION" YMIR_BOOTSTRAP_VERSION "$GYC"
git -C "$YRUNTIME" diff --quiet || commit "$YRUNTIME" "chore: build with gyc $GYC"
if [ "$current_midgard" != "$MIDGARD" ]; then
  # Only the `version` above the first [table] header is the package's own.
  awk -v new="$MIDGARD" '
    !done && /^[[:space:]]*\[/ { done = 1 }
    !done && /^[[:space:]]*version[[:space:]]*=/ { sub(/"[^"]*"/, "\"" new "\""); done = 1 }
    { print }
  ' "$YRUNTIME/gyllir.toml" > "$WORK/gyllir.toml"
  cat "$WORK/gyllir.toml" > "$YRUNTIME/gyllir.toml"
  commit "$YRUNTIME" "chore: bump version $current_midgard -> $MIDGARD"
fi
if ! has_commits "$YRUNTIME"; then
  MIDGARD_BRANCH="$(default_branch "$YRUNTIME")"
  echo "yruntime's $MIDGARD_BRANCH already declares midgard $MIDGARD built with gyc $GYC: gymir bundles it as is."
fi

set_setting "$GYMIR/YMIR_VERSION" GYC_VERSION "$GYC"
set_setting "$GYMIR/YMIR_VERSION" YMIR_BOOTSTRAP_VERSION "$COMPILER"
set_setting "$GYMIR/YMIR_VERSION" YMIR_BOOTSTRAP_MIDGARD_VERSION "$COMPILER_MIDGARD"
set_setting "$GYMIR/YMIR_VERSION" GCC_VERSION "$GCC"
set_setting "$GYMIR/YMIR_VERSION" GYLLIR_VERSION "$GYLLIR"
set_setting "$GYMIR/YMIR_VERSION" MIDGARD_BRANCH "$MIDGARD_BRANCH"
git -C "$GYMIR" diff --quiet || commit "$GYMIR" "chore: prepare $GYC"

GYLLIR_RELEASES="$(git ls-remote --tags --refs "$GYLLIR_REPO" | sed 's|.*refs/tags/||' | grep -E "$SEMVER" | sort -V | paste -sd, -)"
STAGES_WRITTEN="$(python3 "$TOOLS/cd_suite_stages.py" "$CD_SUITE/amd64/deb" "$GYMIR" "$YRUNTIME" "$GYLLIR_RELEASES")"
CD_VERSIONS=()
for s in $STAGES_WRITTEN; do CD_VERSIONS+=("${s#bootstrap_v}"); done
if [ "${#CD_VERSIONS[@]}" -gt 0 ]; then
  CD_LAST="${CD_VERSIONS[-1]}"
  CD_BRANCH="${CD_KEY:+$CD_KEY-}add-$(IFS=-; echo "${CD_VERSIONS[*]}")"
  ! has_branch "$CD_SUITE" "$CD_BRANCH" || die "$(slug "$CD_SUITE") already has a branch $CD_BRANCH"
  if command -v uv >/dev/null; then
    (cd "$CD_SUITE/amd64/deb" && uv run --quiet python -m scripts.check_version_matrix && uv run --quiet python -m scripts.check_remote_tags) \
      || die "CD_suite's version checks reject the new stages"
  else
    warn "uv is missing: CD_suite's version checks were not run"
  fi
  commit "$CD_SUITE" "feat: add $(human_join "${CD_VERSIONS[@]}") in the bootstrap chain"
fi

# ---------------------------------------------------------------------------------------------
# The pull requests
# ---------------------------------------------------------------------------------------------

show() { # show <label> <repo> <branch>
  has_commits "$2" || return 0
  echo
  echo "=============== $1: $3"
  git -C "$2" --no-pager log --reverse -p --format='--- %s' "origin/$(default_branch "$2")..HEAD"
}
[ "$MIDGARD_BRANCH" = "$(default_branch "$YRUNTIME")" ] || show yruntime "$YRUNTIME" "$MIDGARD_BRANCH"
show gymir "$GYMIR" "$GYMIR_BRANCH"
[ "${#CD_VERSIONS[@]}" = 0 ] || show CD_suite "$CD_SUITE" "$CD_BRANCH"
echo

if [ "$DRY_RUN" = 1 ]; then
  echo "Dry run: nothing pushed."
  exit 0
fi
ask CONFIRM "Push these branches and open their pull requests? (y/n)" "n" '^[yn]$'
[ "$CONFIRM" = y ] || { echo "Nothing pushed."; exit 0; }

# open_pr <repo> <branch> <title> <body>: pushes HEAD as <branch>, prints the PR url.
open_pr() {
  git -C "$1" push --quiet origin "HEAD:refs/heads/$2"
  gh pr create --repo "$(slug "$1")" --base "$(default_branch "$1")" --head "$2" --title "$3" --body "$4"
}

MIDGARD_LINK="\`$MIDGARD_BRANCH\`"
if [ "$MIDGARD_BRANCH" != "$(default_branch "$YRUNTIME")" ]; then
  MIDGARD_PR="$(open_pr "$YRUNTIME" "$MIDGARD_BRANCH" "[$MID_KEY][chore] Build with gyc $GYC" \
"Midgard $MIDGARD, built with gyc $GYC.

gymir's $GYC release bundles this branch (its \`MIDGARD_BRANCH\`), then dispatches this repository's release, which tags $MIDGARD and merges this pull request. Whatever the $GYC compiler requires of the std lands here.")"
  echo "yruntime: $MIDGARD_PR"
  MIDGARD_LINK="[\`$MIDGARD_BRANCH\`]($MIDGARD_PR)"
fi

# The check gymir's own release runs on its MIDGARD_BRANCH, now that the branch and its PR exist.
bash "$GYMIR/.github/scripts/midgard-branch.sh" --branch="$MIDGARD_BRANCH" --expect-gyc="$GYC" >/dev/null \
  || die "gymir's release would reject yruntime's $MIDGARD_BRANCH - gymir's branch was not pushed"

if has_commits "$GYMIR"; then
  GYMIR_PR="$(open_pr "$GYMIR" "$GYMIR_BRANCH" "[$GYC_KEY] Prepare $GYC" \
"Releases gyc $GYC.

| \`YMIR_VERSION\` | |
|---|---|
| \`GYC_VERSION\` | $GYC |
| \`YMIR_BOOTSTRAP_VERSION\` | $COMPILER |
| \`YMIR_BOOTSTRAP_MIDGARD_VERSION\` | $COMPILER_MIDGARD |
| \`GCC_VERSION\` | $GCC |
| \`GYLLIR_VERSION\` | $GYLLIR |
| \`MIDGARD_BRANCH\` | $MIDGARD_LINK, midgard $MIDGARD |")"
  echo "gymir: $GYMIR_PR"
fi

if [ "${#CD_VERSIONS[@]}" -gt 0 ]; then
  CD_PR="$(open_pr "$CD_SUITE" "$CD_BRANCH" "${CD_KEY:+[$CD_KEY] }Extend the bootstrap chain to $CD_LAST" \
"Adds $(human_join "${CD_VERSIONS[@]}") to the bootstrap chain.")"
  echo "CD_suite: $CD_PR"
fi
