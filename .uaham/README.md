# UaHamAward fork of WSJT-X

A build of WSJT-X with two additions for operators taking part in
[uahamaward.com](https://uahamaward.com) awards. Everything else is upstream
WSJT-X, unchanged.

**This is not an official WSJT-X release.** Report problems with anything
described here to this repository, never to the WSJT-X team.

- Upstream: <https://github.com/WSJTX/wsjtx> (`upstream` remote)
- Base: `v3.0.2`
- Version marker: builds report `X.Y.Z-uahamN`, e.g. `3.0.2-uaham1`

## What it adds

### A country filter that works by DXCC entity

Settings → **UaHam Filter**. Pick any number of countries from the cty.dat
list and choose one of:

- **Hide the countries ticked below** — a hunter who does not want to see a
  particular country at all.
- **Show only the countries ticked below** — a hunter chasing specific
  entities, for whom everything else is noise.

A hidden station is hidden from Band Activity, from Rx Frequency and from
Active Stations, and auto-sequencing will not answer it even when it calls you
directly. ALL.TXT keeps every decode, and programs reading WSJT-X over UDP —
JTAlert, GridTracker — still receive them all and apply their own filters.

In *show only* mode a decode whose country cannot be established (free text, an
unresolved `<...>` callsign) is hidden as well.

**Why not the existing Filters tab.** That one holds twelve fixed text boxes
per list and compares them against a word of the message with `startsWith()`,
which answers what a message looks like rather than who sent it. "Only Japan"
cannot be written as twelve prefixes — JA, JE…JS, 7J…7N, 8J…8N is already more
than twelve — and a compound callsign gets through anyway. cty.dat knows the
answer exactly, so this asks cty.dat.

Countries are stored in the settings file by primary prefix (`UR`, `JA`, `K`),
never by cty.dat's internal entity id: those ids are handed out in file order
and are renumbered by every cty.dat update, so a setting that remembered one
would quietly start naming a different country.

### A direct connection to uahamaward.com

Settings → **UaHam Site**. WSJT-X serves a WebSocket on `127.0.0.1` that the
site's award log pages connect to, and hands over every logged contact the
moment it is logged — whether the operator pressed Log QSO or auto-sequencing
did it for them.

This replaces `uaham-bridge`, the separate program that used to sit between
WSJT-X's UDP output and the browser. The protocol is deliberately identical —
same default port 8080, same greeting, same message names — so the site needs
no change to accept contacts from here, and the two can be swapped either way.

With no log page open nothing is sent, and logging behaves exactly as it always
does. The connection is a copy for somebody watching, never a step in logging a
contact.

The port is bound on the loopback address only: the browser reaches it at
127.0.0.1, and on Windows binding a wildcard address is what raises the
firewall prompt an operator has no way to judge. A busy port is stepped over —
the server takes the first free one in the twenty above the one asked for,
which is the same window the site's browser-side probe searches.

### Main window

A **UaHam** tab beside «1» and «2» reports the filter's mode, how many decodes
it has hidden, whether a browser is connected and on which port, and how many
QSOs have gone out, with one button that clears both counters. The same in two
words each appears in the status bar, and both indicators hide themselves when
the feature behind them is off.

### One upstream bug fixed on the way

`filtered` was cleared once per call to `readFromStdout`, outside the loop over
decoded lines, while `blockUDP` and `block_right_display` beside it were
cleared per line. One filtered decode therefore left the flag set for every
later decode in the same batch, hiding them all and suppressing auto-sequencing
for the rest of the period. It is now reset per decode, as its neighbours
always were.

## Building

The toolchain lives in a Docker image built from the package list in
`.github/workflows/build-linux.yml`, so a local build uses the same compiler,
the same Qt and — the part that catches people out — the same statically
linked Hamlib 4.7.0 as the released binaries.

```sh
.uaham/docker/build.sh            # configure if needed, then build
.uaham/docker/build.sh --fresh    # throw the build directory away first
.uaham/docker/build.sh --tests    # build and run the ctest suite
```

Output lands in `build/linux`. The first run builds the image, which takes
about a quarter of an hour; after that an incremental build is seconds.

## Releasing

Artifacts for every platform are built by the workflows inherited from
upstream. A release is one pushed tag:

```sh
git tag build/v3.0.2-uaham1
git push origin build/v3.0.2-uaham1
```

The tag's fork marker must match `WSJT_FORK_TAG` in `CMakeLists.txt` — the tag
names the files, `CMakeLists.txt` names what is inside them, and the release
job refuses to run if the two disagree. The numeric part must equally match
`project(VERSION)`, which is upstream's own check.

Tags of the form `build/vX.Y.Z` and `build/vX.Y.Z-rcN` still mean what they
mean upstream, and are not for this fork's releases.

## Merging a new WSJT-X release

```sh
git fetch upstream --tags
git merge v3.0.3          # or whichever tag
```

Almost everything this fork adds lives in `UaHam/`, which upstream will never
touch. The deliberate exceptions, and the whole of the contact surface:

| File | Change |
|---|---|
| `CMakeLists.txt` | `WSJT_FORK_TAG`, four source files added to `wsjt_qt_CXXSRCS` |
| `Configuration.hpp` / `.cpp` | settings for both tabs; the tabs themselves are added from code, so `Configuration.ui` is untouched |
| `widgets/mainwindow.h` / `.cpp` | the filter check, the status tab and indicators, the hook in `acceptQSO` |
| `logbook/AD1CCty.hpp` / `.cpp` | `entities()`, listing the whole cty.dat table |
| `widgets/about.cpp` | the modified-build notice GPLv3 §5(a) asks for |
| `.github/workflows/release.yml` | accepts `-uahamN` tags |

After merging, bump `WSJT_FORK_TAG` and tag a release for the new base.
