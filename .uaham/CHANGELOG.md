# Changelog — UaHamAward fork of WSJT-X

Only this fork's changes. Upstream's own history is in `NEWS` and
`Release_Notes.txt`; nothing here duplicates them.

Written for whoever picks this up next, including an agent in a fresh session:
each entry says what changed and, where it matters, why it is that way rather
than the obvious alternative.

---

## 3.0.2-uaham2 — 2026-08-14

### QRZ.com callsign lookup

A «Call info» tab shows the name, address, country and grid of whoever is in
the DX Call box. Credentials under Settings → QRZ; with none, the tab says so
and no request is ever made.

Ported from WSJT-Z, which does this inline in its main window. Note WSJT-Z
also carries a `qrzlookup.cpp` that is **not in its build and that nothing
constructs** — dead code; the working implementation was the one in
`mainwindow.cpp`. Two things WSJT-Z loses are handled here: a lookup made
before the session exists is remembered and repeated rather than silently
dropped, and an expired session is renewed once instead of surfacing as a
failure.

The DX Call box drives it because that is the one place that always holds the
station being worked, however it got there. Unchanged text and anything
shorter than three characters make no request.

### Ukrainian interface

`translations/wsjtx_uk.ts`, about 1250 of 1650 strings, plus a Language tab.
`main.cpp` reads the saved choice before Qt installs translators, so the
setting takes effect on the next start — and the tab says so, because
retranslating a running WSJT-X would mean re-running `retranslateUi` across
every window and a half-translated one is worse than an honest sentence.
`--language` on the command line still wins, which is what makes a wrong
setting recoverable without editing an ini file.

### Author named in the interface

The window title and the About box name the build's author, UR3PKI.

---

## 3.0.2-uaham1 — 2026-08-13

First release. Nine installers: Windows, macOS ×2, Linux ×2 in three formats.

### Country filter

Settings → UaHam Filter. Any number of DXCC entities, in two modes: hide the
chosen ones, or show only those. A hidden station is hidden from Band
Activity, Rx Frequency and Active Stations, and auto-sequencing will not
answer it even when it calls directly. ALL.TXT and the UDP feed other programs
read stay complete.

Kept separate from WSJT-X's own Filters tab on purpose: that one compares
twelve fixed text boxes against a word of the message with `startsWith()`,
which answers what a message looks like rather than who sent it. "Only Japan"
cannot be written as twelve prefixes.

### Direct connection to uahamaward.com

Settings → UaHam Site. A WebSocket on 127.0.0.1 hands every logged contact to
the site as it is logged, replacing the separate `uaham-bridge` program. The
protocol is the bridge's exactly, so the site needed no change.

### Fixed while in there

`filtered` was cleared once per call to `readFromStdout` rather than per
decode, so one filtered decode hid every later decode in the same period and
suppressed auto-sequencing with it. Worth reporting upstream.

### Fork marking

`WSJT_FORK_TAG` appends the marker to the version (`3.0.2-uaham1`), the About
box names the build and its author, and the window title carries it too.
GPLv3 §5(a) asks for the notice; the practical reason is that a bug report
about the country filter must not land on the WSJT-X team.

### CI fixes needed to build at all

Four, all in inherited workflows, all worth upstreaming:

- **Qt source download died at 96%** with `curl: (92)`. `--retry 5` was already
  there and never spent an attempt: curl does not retry a transfer that breaks
  mid-stream. Added `--retry-all-errors`, `-C -` and `--http1.1`.
- **Hamlib's macOS binaries had no header padding**, so the bundling step could
  not add an rpath to `rigctl-wsjtx`. `-headerpad_max_install_names` was passed
  to the WSJT-X CMake build but not to Hamlib's autotools build.
- **Artifact names lacked the fork marker** in the CI path: `release.yml` takes
  its version from the tag, `ci.yml` from `CMakeLists.txt`. Both now append
  `WSJT_FORK_TAG`.
- **The release job force-pushed to `WSJTX/wsjtx`.** Right for the repository
  that publishes WSJT-X, catastrophic in a fork. Removed, not re-guarded.

### Release page

Assets were filtered by extension, which swept in three dozen utility binaries
and a bare `wsjtx.exe` — 46 files, nine of them the ones anybody wanted. Now
filtered by release name prefix as well.

---

## Base

Forked from `WSJTX/wsjtx` at `v3.0.2` (2026-06-18).

An earlier attempt forked `saitohirga/WSJT-X`, a SourceForge mirror that
stopped at 2.7.0-rc4 in March 2024 — two years and a major version behind. If
a future merge looks strange, check `git remote -v` says `WSJTX/wsjtx`.
