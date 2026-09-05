#!/usr/bin/env python3
"""Mirror pull request state onto Plane work items.

Reads the pull_request event payload and takes the work item from a
leading [IDENT-N] tag on the PR title, e.g. "[BAL-47][ci] Log". That tag
is the only source, so a body mentioning another item cannot attach the
PR to it. For that one item it:

  * attaches the PR url as a link on the work item (idempotent),
  * comments when the PR opens, closes or merges,
  * moves the item to In Progress when the PR opens and to Done when it
    merges (the first state of the started and completed groups).

Standard library only, so the workflow needs no pip install.
"""

import json
import os
import re
import sys
import urllib.error
import urllib.request

BASE = os.environ["PLANE_BASE_URL"].rstrip("/")
SLUG = os.environ["PLANE_WORKSPACE_SLUG"]
TOKEN = os.environ["PLANE_API_TOKEN"]

API = f"{BASE}/api/v1/workspaces/{SLUG}"


def request(method, path, payload=None):
    """Call the Plane API. Returns (status, decoded body or None)."""
    url = path if path.startswith("http") else f"{API}{path}"
    data = json.dumps(payload).encode() if payload is not None else None
    req = urllib.request.Request(url, data=data, method=method)
    req.add_header("X-Api-Key", TOKEN)
    if data:
        req.add_header("Content-Type", "application/json")
    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            body = resp.read()
            return resp.status, json.loads(body) if body else None
    except urllib.error.HTTPError as exc:
        return exc.code, None


def paginate(path):
    """Yield every result across a paginated list endpoint."""
    while path:
        status, body = request("GET", path)
        if status != 200 or body is None:
            return
        if isinstance(body, list):
            yield from body
            return
        yield from body.get("results", [])
        if not body.get("next_page_results") or not body.get("next_cursor"):
            return
        path = f"{path.split('?')[0]}?cursor={body['next_cursor']}"


def known_identifiers():
    """Map project identifier -> project id, so we only chase real prefixes."""
    return {p["identifier"].upper(): p["id"] for p in paginate("/projects/")}


# The work item is named by a leading [IDENT-N] tag on the PR title, and
# nowhere else. Branch names and body prose are deliberately not scanned:
# a body that merely mentions another item ("same change as BAL-47") must
# not attach this PR to it.
TITLE_REF = re.compile(r"^\s*\[([A-Za-z][A-Za-z0-9]{1,19})-(\d+)\]")


def find_reference(pr, identifiers):
    """Return the work item this PR is for, or None."""
    match = TITLE_REF.match(pr.get("title") or "")
    if not match:
        return None
    prefix, number = match.group(1).upper(), match.group(2)
    if prefix not in identifiers:
        return None
    return f"{prefix}-{number}"


def resolve(ref):
    """Look a work item up by its human identifier."""
    status, item = request("GET", f"/work-items/{ref}/")
    if status == 200:
        return item
    if status != 404:
        print(f"  ! {ref}: lookup failed with HTTP {status}", file=sys.stderr)
    return None


def ensure_link(item, pr):
    """Attach the PR url once; repeat runs are no-ops."""
    base = f"/projects/{item['project']}/work-items/{item['id']}/links/"
    for link in paginate(base):
        if link.get("url") == pr["html_url"]:
            return "already linked"
    status, _ = request(
        "POST",
        base,
        {"url": pr["html_url"], "title": f"PR #{pr['number']}: {pr['title']}"},
    )
    return "linked" if status in (200, 201) else f"link failed (HTTP {status})"


def comment(item, html):
    path = f"/projects/{item['project']}/work-items/{item['id']}/comments/"
    status, _ = request("POST", path, {"comment_html": html})
    return status in (200, 201)


def move_state(item, group):
    """Move the item into the earliest state of the given group.

    Groups are Plane's fixed five: backlog, unstarted, started, completed,
    cancelled. A project may hold several states per group, so order by the
    sequence the board shows and take the first.
    """
    candidates = sorted(
        (s for s in paginate(f"/projects/{item['project']}/states/") if s.get("group") == group),
        key=lambda s: s.get("sequence", 0),
    )
    if not candidates:
        return f"no {group} state in project"

    state = candidates[0]
    if item.get("state") == state["id"]:
        return f"already {state['name']}"

    status, _ = request(
        "PATCH",
        f"/projects/{item['project']}/work-items/{item['id']}/",
        {"state": state["id"]},
    )
    if status == 200:
        return f"state -> {state['name']}"
    return f"state change failed (HTTP {status})"


def target_group(action, merged):
    """Which state group this event should put the work item in, if any."""
    if action in ("opened", "reopened", "ready_for_review"):
        return "started"
    if action == "closed" and merged:
        return "completed"
    return None


def describe(pr, action, merged):
    """The comment body for this event, or None to stay quiet."""
    link = f'<a href="{pr["html_url"]}">#{pr["number"]} {pr["title"]}</a>'
    author = pr["user"]["login"]
    if action in ("opened", "reopened"):
        return f"<p>Pull request {link} opened by {author}.</p>"
    if action == "closed":
        if merged:
            return f"<p>Pull request {link} merged into <code>{pr['base']['ref']}</code>.</p>"
        return f"<p>Pull request {link} closed without merging.</p>"
    if action == "ready_for_review":
        return f"<p>Pull request {link} is ready for review.</p>"
    return None


def main():
    with open(os.environ["GITHUB_EVENT_PATH"]) as handle:
        event = json.load(handle)

    pr = event["pull_request"]
    action = event["action"]
    merged = bool(pr.get("merged"))

    identifiers = known_identifiers()
    if not identifiers:
        print("Could not list Plane projects - check the token and base url.", file=sys.stderr)
        return 1

    ref = find_reference(pr, identifiers)
    if ref is None:
        print("PR title does not start with a known [IDENT-N] tag - nothing to do.")
        return 0

    item = resolve(ref)
    if item is None:
        print(f"  - {ref}: no such work item, skipped")
        return 0

    notes = [ensure_link(item, pr)]
    body = describe(pr, action, merged)
    if body and comment(item, body):
        notes.append("commented")
    group = target_group(action, merged)
    if group:
        notes.append(move_state(item, group))
    print(f"  - {ref}: {', '.join(notes)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
