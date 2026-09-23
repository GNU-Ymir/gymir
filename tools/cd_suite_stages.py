#!/usr/bin/env python3
"""Adds to CD_suite's bootstrap chain the gyc releases it does not list yet.

usage: cd_suite_stages.py <CD_suite amd64/deb> <gymir> <yruntime> <gyllir releases, comma separated>

The chain holds one stage per minor, its last patch, since 1.N.* compiles from the last 1.(N-1).*:
a newer patch of the last stage's minor replaces it, and each newer minor is appended. The midgard
each stage bundles and the gyllir it builds are asked on the terminal. Edits utils/versions.py and
config.yaml in place, and prints the name of each stage written.
"""

import re
import subprocess
import sys
from pathlib import Path

SEMVER = re.compile(r"^\d+\.\d+\.\d+$")


def key(version: str) -> tuple[int, ...]:
    return tuple(int(part) for part in version.split("."))


def git(repo: str, *args: str) -> str:
    return subprocess.run(["git", "-C", repo, *args], capture_output=True, text=True).stdout


def releases(repo: str) -> list[str]:
    return sorted((t for t in git(repo, "tag", "-l").split() if SEMVER.match(t)), key=key)


def setting(repo: str, ref: str, name: str) -> str | None:
    values = re.findall(rf"^{name}=(.*)$", git(repo, "show", f"{ref}:YMIR_VERSION"), re.M)
    return values[-1].strip() if values else None


def bundled_midgard(gymir: str, yruntime: str, gyc: str) -> str:
    """The midgard a gyc release bundles: named by its YMIR_VERSION until MIDGARD_BRANCH replaced
    it, and since then the yruntime release built with that gyc."""
    named = setting(gymir, gyc, "MIDGARD_VERSION")
    if named:
        return named
    built = [t for t in releases(yruntime) if setting(yruntime, t, "YMIR_BOOTSTRAP_VERSION") == gyc]
    return built[-1] if built else ""


def ask(tty, question: str, default: str) -> str:
    print(f"{question} [{default}]: ", end="", file=sys.stderr, flush=True)
    return tty.readline().strip() or default


def block(text: str, name: str) -> str:
    match = re.search(rf'^    "{re.escape(name)}":.*?^    \),\n', text, re.M | re.S)
    if match is None:
        sys.exit(f"utils/versions.py: no block for the stage {name}")
    return match.group(0)


def ubuntu_name(versions, value: str) -> str:
    names = [n for n in vars(versions) if n.startswith("UBUNTU_FOR_GCC") and getattr(versions, n) == value]
    return names[-1] if names else f'"{value}"'


def render(name: str, comments: list[str], prev_gyc: str, prev_gyllir: str, v, ubuntu: str, gyllir: str) -> str:
    lines = [
        f'    "{name}": BootstrapStage (',
        *comments,
        f'        prev_gyc="{prev_gyc}",',
        f'        prev_gyllir="{prev_gyllir}",',
        f'        versions=GycVersions (compiler="{v.compiler}", target="{v.target}", ymir="{v.ymir}", '
        f'bootstrap="{v.bootstrap}", midgard="{v.midgard}"),',
        f"        ubuntu_version={ubuntu},",
    ]
    if gyllir:
        lines.append(
            f'        gyllir=GyllirSpec(gyc="{v.target_major}_{v.bootstrap}", compile_with="{v.target_major}", '
            f'gyllir_version="{gyllir}", ubuntu_version={ubuntu}, prev_gyllir="{prev_gyllir}"),'
        )
    lines.append("    ),")
    return "\n".join(lines) + "\n"


def main() -> int:
    deb, gymir, yruntime = sys.argv[1:4]
    gyllirs = [t for t in sys.argv[4].split(",") if SEMVER.match(t)]
    sys.path.insert(0, deb)
    from utils import versions as V

    last_name = list(V.STAGES)[-1]
    last = V.STAGES[last_name]
    if not isinstance(last, V.BootstrapStage):
        sys.exit(f"utils/versions.py: the last stage {last_name} is not a bootstrap stage")

    newest: dict[tuple[int, ...], str] = {}
    for tag in releases(gymir):
        if key(tag) > key(last.versions.bootstrap):
            newest[key(tag)[:2]] = tag
    if not newest:
        return 0

    replaced = newest.pop(key(last.versions.bootstrap)[:2], None)
    chain = ([replaced] if replaced else []) + [newest[minor] for minor in sorted(newest)]
    produced_gyllir = [
        s.gyllir.gyllir_version
        for n, s in V.STAGES.items()
        if s.gyllir is not None and not (replaced and n == last_name)
    ][-1]

    versions_py = Path(deb, "utils", "versions.py")
    config = Path(deb, "config.yaml")
    text = versions_py.read_text()
    base_block = block(text, last_name)
    comments = re.findall(r"^        #.*$", base_block.split("prev_gyc=")[0], re.M)
    ubuntu = ubuntu_name(V, last.ubuntu_version)
    appended: list[tuple[str, str]] = []
    base = last

    with open("/dev/tty") as tty:
        for i, gyc in enumerate(chain):
            if gyc == replaced:
                prev_gyc, prev_gyllir = last.prev_gyc, last.prev_gyllir
            else:
                prev_gyc = f"{base.versions.target_major}_{base.versions.bootstrap}"
                prev_gyllir = produced_gyllir

            midgard = ask(tty, f"CD_suite: midgard bundled by gyc {gyc}", bundled_midgard(gymir, yruntime, gyc))
            newer = [g for g in gyllirs if not SEMVER.match(produced_gyllir) or key(g) > key(produced_gyllir)]
            default = newer[-1] if newer and i == len(chain) - 1 else ""
            gyllir = ask(tty, f"CD_suite: gyllir built with gyc {gyc} (empty for none)", default)

            v = V.GycVersions(compiler=base.versions.compiler, target=base.versions.target,
                              ymir=gyc, bootstrap=gyc, midgard=midgard)
            name = f"bootstrap_v{gyc}"
            rendered = render(name, comments, prev_gyc, prev_gyllir, v, ubuntu, gyllir)
            if gyc == replaced:
                text = text.replace(base_block, rendered)
            else:
                appended.append((name, rendered))

            if gyllir:
                produced_gyllir = gyllir
            base = V.BootstrapStage(prev_gyc=prev_gyc, prev_gyllir=prev_gyllir, versions=v,
                                    ubuntu_version=last.ubuntu_version)

    end = text.rstrip().rindex("}")
    text = text[:end] + "".join(r for _, r in appended) + text[end:]
    versions_py.write_text(text)

    lines = config.read_text().splitlines(keepends=True)
    if replaced:
        lines = [f"- bootstrap_v{replaced}\n" if l.strip() == f"- {last_name}" else l for l in lines]
    listed = max(i for i, l in enumerate(lines) if l.startswith("- "))
    lines[listed + 1:listed + 1] = [f"- {n}\n" for n, _ in appended]
    config.write_text("".join(lines))

    for gyc in chain:
        print(f"bootstrap_v{gyc}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
