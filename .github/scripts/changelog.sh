#!/usr/bin/env bash
# Prints a markdown changelog for the release workflows: one entry per merged pull request
# between the previous release tag and the ref being released. The commits inside a PR are
# implementation detail and are not listed - only the PR title is.
#
# A PR title must read `[YMI-XXX][kind] Log`, where `YMI-XXX` is the Linear issue and `[kind]` is
# optional, defaulting to a feature. Recognised kinds: feat/feature, fix, perf, refactor, doc(s),
# test(s), chore/ci/build/style, breaking. A PR whose title does not follow that format - no
# issue key, or a kind that is not in the list - is left out of the changelog entirely; each skip
# is reported on stderr so it shows up in the release job log.
#
# Usage:
#   changelog.sh [--since <ref>] [--to <ref>]   # the changelog itself
#   changelog.sh [--to <ref>] --print-base      # just the resolved base tag (empty if none)
#
# With no --since, the base is the most recent `<major>.<minor>.<patch>` tag reachable from --to
# (`preview` and `cxx` are not matched), so this works for both release.yml - where the tag being
# created does not exist yet - and release-preview.yml, whose own rolling tag is skipped.
#
# @warning: needs the full history, so the workflow must check out with `fetch-depth: 0`.
set -euo pipefail

TO="HEAD"
FROM=""
FROM_SET=0
PRINT_BASE=0

while [ $# -gt 0 ]; do
  case "$1" in
    --since) FROM="${2-}"; FROM_SET=1; shift 2 ;;
    --to) TO="${2-}"; shift 2 ;;
    --print-base) PRINT_BASE=1; shift ;;
    *) echo "changelog.sh: unknown argument: $1" >&2; exit 1 ;;
  esac
done

if [ "$FROM_SET" -eq 0 ]; then
  FROM="$(git describe --tags --abbrev=0 --match '[0-9]*.[0-9]*.[0-9]*' "$TO" 2>/dev/null || true)"
fi

if [ "$PRINT_BASE" -eq 1 ]; then
  echo "$FROM"
  exit 0
fi

if [ -n "$FROM" ]; then
  RANGE="${FROM}..${TO}"
else
  # No release tag yet: the changelog covers the whole history.
  RANGE="$TO"
fi

# --first-parent so only the merges made *into* the released branch are walked, never the merges
# a feature branch took from master along the way. GitHub puts the PR number in the merge subject
# and the PR title in the body, hence \037 (unit separator) between the fields and \036 (record
# separator) between commits - neither can appear in a commit message.
git log --first-parent --merges --pretty=format:'%h%x1f%s%x1f%b%x1e' "$RANGE" | awk '
BEGIN {
  RS = "\036"
  FS = "\037"

  n = split("breaking feat fix perf refactor docs test chore", order, " ")
  title["breaking"] = "Breaking changes"
  title["feat"]     = "Features"
  title["fix"]      = "Bug fixes"
  title["perf"]     = "Performance"
  title["refactor"] = "Refactoring"
  title["docs"]     = "Documentation"
  title["test"]     = "Tests"
  title["chore"]    = "Chores"

  kinds["feat"] = "feat"; kinds["feature"] = "feat"
  kinds["fix"] = "fix"; kinds["bugfix"] = "fix"; kinds["bug"] = "fix"
  kinds["perf"] = "perf"
  kinds["refactor"] = "refactor"
  kinds["doc"] = "docs"; kinds["docs"] = "docs"
  kinds["test"] = "test"; kinds["tests"] = "test"
  kinds["chore"] = "chore"; kinds["ci"] = "chore"; kinds["build"] = "chore"; kinds["style"] = "chore"
  kinds["breaking"] = "breaking"; kinds["break"] = "breaking"
}

{
  sha = $1; subj = $2; msg = $3
  gsub(/^[\n\r]+/, "", sha)
  if (sha == "") next

  # Only pull request merges: a plain `Merge branch ...` is not a change of its own.
  if (!match(subj, /#[0-9]+/)) next
  pr = substr(subj, RSTART, RLENGTH)

  # GitHub writes the PR title as the first non-empty line of the merge commit body.
  head = ""
  cnt = split(msg, lines, "\n")
  for (i = 1; i <= cnt; i++) {
    line = lines[i]
    gsub(/^[ \t\r]+|[ \t\r]+$/, "", line)
    if (line != "") { head = line; break }
  }
  if (head == "") next

  # `[YMI-XXX][kind] Log`, the kind being optional.
  rest = head; ticket = ""; kind = ""; text = head

  if (match(rest, /^\[[^]]+\]/)) {
    candidate = substr(rest, RSTART + 1, RLENGTH - 2)
    if (candidate ~ /^[A-Za-z]+-[0-9]+$/) {
      ticket = toupper(candidate)
      rest = substr(rest, RSTART + RLENGTH)
      sub(/^[ \t]+/, "", rest)

      if (match(rest, /^\[[^]]*\]/)) {
        kind = tolower(substr(rest, RSTART + 1, RLENGTH - 2))
        rest = substr(rest, RSTART + RLENGTH)
        sub(/^[ \t]+/, "", rest)
      }

      text = rest
    }
  }

  # Off-format titles are dropped rather than guessed at, but never silently: the skip goes to
  # stderr so the release job log says which PRs were left out of the notes.
  if (ticket == "") {
    print "changelog: ignoring " pr " (" sha "), title does not start with an issue key: " head > "/dev/stderr"
    skipped++
    next
  }

  if (kind == "") {
    # A kind-less title is a feature, as agreed in the PR title convention.
    type = "feat"
  } else if (kind in kinds) {
    type = kinds[kind]
  } else {
    print "changelog: ignoring " pr " (" sha "), unknown kind [" kind "]: " head > "/dev/stderr"
    skipped++
    next
  }

  if (text == "") {
    print "changelog: ignoring " pr " (" sha "), title has no text after its tags: " head > "/dev/stderr"
    skipped++
    next
  }

  body[type] = body[type] "- **" ticket "** " text " (" pr ")\n"
  count++
}

END {
  if (count == 0) {
    if (skipped > 0) {
      print "_No pull request with a conforming title in this range._"
    } else {
      print "_No pull requests merged._"
    }
    exit
  }

  first = 1
  for (i = 1; i <= n; i++) {
    s = order[i]
    if (s in body) {
      if (!first) print ""
      printf "### %s\n", title[s]
      printf "%s", body[s]
      first = 0
    }
  }
}
'
