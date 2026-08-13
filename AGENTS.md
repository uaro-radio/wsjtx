# WSJT-X — UaHamAward fork

A fork of WSJT-X carrying two features for operators taking part in
[uahamaward.com](https://uahamaward.com) awards, plus a Ukrainian interface.
Everything else is upstream WSJT-X, unchanged.

**Read this before touching anything.** Most of what is written here was
learned the expensive way, and every paragraph exists because something went
wrong without it.

---

## The three repositories, and how they relate

| | What it is | Where |
|---|---|---|
| **This repo** | Fork of WSJT-X at `v3.0.2` | `uaro-radio/wsjtx` |
| **WSJT-Z fork** | The same two features ported to WSJT-Z | `uaro-radio/wsjt-z` |
| **The platform** | The award site the two talk to | `PetroOstapuk/UaHamAward` (local: `~/Documents/Projects/UaHamAward`) |

The two forks are **siblings, not a chain**: a change made here is ported to
WSJT-Z by hand, and the reverse. They share the `UaHam/` directory almost
verbatim; when you change something there, ask whether the other fork needs
the same change. It usually does.

**Upstreams.** This tree's `upstream` remote is `WSJTX/wsjtx` — the real home
of WSJT-X since February 2026. The old SourceForge git and the
`saitohirga/WSJT-X` GitHub mirror both stopped at 2.7.0-rc4 in March 2024;
an earlier fork of this project was made from that mirror and had to be
abandoned. WSJT-Z's upstream is `sq9fve/wsjt-z`.

### What replaced what

The site used to need a separate program, `uaham-bridge` (Go, still in
`UaHamAward/wsjtx-bridge/`), that received WSJT-X's UDP packets and repeated
them to the browser. **The fork removes that moving part**: WSJT-X now serves
the WebSocket itself. The protocol is deliberately identical, so the site
cannot tell the two apart and needed no change at all — see the contract
below before altering anything about it.

---

## What the fork adds

All of it lives in `UaHam/`, which upstream will never touch.

| File | Responsibility |
|---|---|
| `UaHam/CountryFilter.hpp` | Header-only. Off / Block / Only, decided on a DXCC primary prefix |
| `UaHam/SiteServer.{hpp,cpp}` | The WebSocket endpoint the site connects to |
| `UaHam/QsoPayload.{hpp,cpp}` | Logged ADIF → the JSON the site expects |
| `UaHam/SettingsTabs.{hpp,cpp}` | The four Settings tabs, built in code |
| `UaHam/StatusWidget.{hpp,cpp}` | The «UaHam» tab in the main window |
| `UaHam/QrzLookup.{hpp,cpp}` | QRZ.com XML callbook session and query |
| `UaHam/CallInfoWidget.{hpp,cpp}` | The «Call info» tab |

### Contact points outside `UaHam/`

This is the whole surface an upstream merge can conflict with. Keep it short.

| File | Why it is touched |
|---|---|
| `CMakeLists.txt` | `WSJT_FORK_TAG`; the `UaHam/*.cpp` sources; `uk` in `LANGUAGES` |
| `Configuration.hpp` / `.cpp` | Settings storage and accessors for everything above |
| `widgets/mainwindow.h` / `.cpp` | The filter check, the tabs, the `acceptQSO` hook, the QRZ trigger |
| `logbook/AD1CCty.hpp` / `.cpp` | `entities()` — the whole cty.dat table, for the country picker |
| `widgets/about.cpp` | The modified-build notice GPLv3 §5(a) asks for |
| `main.cpp` | Reads the saved UI language before translators are installed |
| `translations/wsjtx_uk.ts` | The Ukrainian translation |
| `.github/workflows/{ci,release}.yml` | Fork markers in artifact names; `-uahamN` tags; no push to upstream |

**Settings tabs are added from code, never to `Configuration.ui`.** That file
is five thousand lines of generated XML which upstream edits in most releases.
A tab written into it is a merge conflict in every future version.

---

## Building

```sh
.uaham/docker/build.sh            # configure if needed, then build
.uaham/docker/build.sh --fresh    # throw the build directory away first
```

The image mirrors `.github/workflows/build-linux.yml`: Ubuntu 24.04, Qt5, and
**Hamlib 4.7.0 built from source and linked statically**. A distribution's
libhamlib is not a substitute — a rig backend that differs between a local
build and the released binary produces bug reports nobody can reproduce.

First run builds the image (~15 min). After that an incremental build is
seconds. Output in `build/linux/`.

---

## Releasing

One pushed tag does everything:

```sh
git tag build/v3.0.2-uaham1
git push origin build/v3.0.2-uaham1
```

- The tag's marker must match `WSJT_FORK_TAG` in `CMakeLists.txt`, and its
  numeric base must match `project(VERSION)`. The job refuses the tag
  otherwise: the tag names the files, `CMakeLists.txt` names what is inside
  them, and a file called `uaham2` holding a binary that reports `uaham1`
  helps nobody.
- Nothing is published unless **all five platforms** produced an installer.
- The release description comes from `.uaham/release-notes.md`. Edit that, not
  the workflow. Generated notes alone are a list of commit subjects, which
  says nothing to an operator who came to download a program.
- **The release rebuilds everything.** CI artifacts are not reused, and must
  not be: CI derives its version from `CMakeLists.txt` on the DEVEL channel,
  so its binaries report `3.0.2-devel-uaham1`, and a release must be built
  from the tagged commit to be worth the name.

`build/vX.Y.Z` and `build/vX.Y.Z-rcN` still mean what they mean upstream and
are not for this fork.

---

## Merging a new WSJT-X release

```sh
git fetch upstream --tags
git merge v3.0.3
```

Then re-read the contact-point table above, bump `WSJT_FORK_TAG`, run
`lupdate` (below) for new strings, and tag.

---

## The contract with the site — do not break this

`UaHam/SiteServer` speaks the Go bridge's protocol exactly. The site's
consumer is `UaHamAward/frontend/src/hooks/useLocalBridge.ts`; read it before
changing anything here.

- **Endpoint**: `ws://127.0.0.1:<port>/ws`, default port 8080, stepping to the
  first free port in the twenty above it — the same window the browser probes.
- **Loopback only.** Not a detail: binding a wildcard address on Windows is
  what raises the firewall prompt an operator has no way to judge.
- **On connect**: `{"type":"hello","info":{version,udpPort,wsPort,source}}`.
  `udpPort` is 0 and meaningless here; it is kept because an older page reads
  it without checking. `source` is ours and ignored by the current site.
- **On a logged QSO**: `{"type":"qso_logged","payload":{…}}` with
  `workedCallsign, band, frequency, mode, rstSent, rstRecv, workedGrid,
  workedAt, activatorCallsign, rawAdif`.
- **Frequency is kilohertz**, ADIF states megahertz — `QsoPayload` shifts the
  decimal point rather than multiplying.
- **MFSK submodes are unfolded**: an FT4 contact must not arrive as `MFSK`.
- The version string must not parse as newer than the published bridge
  version, or the site shows a spurious "your bridge is out of date" notice.
  `3.0.2-uaham1` parses as `[3,0,2]`, which is fine.

---

## Gotchas that cost real time

**The `filtered` flag was reset per batch, not per decode.** `readFromStdout`
cleared it once per call, outside the loop over decoded lines, while its
neighbours `blockUDP` and `block_right_display` were cleared per line. One
filtered decode therefore hid every later decode in the same period and
suppressed auto-sequencing with it. Fixed here; the same bug exists in WSJT-Z
and is fixed there too.

**WSJT-X's own filters do not gate the Rx Frequency window**, which is why a
station they hide still appears the moment it calls you. The country filter
gates it separately, and also defeats the "Use filters for Wait and Pounce
only" escape hatch — it was asked to hide a country, so it hides it.

**Auto-sequencing answers a station before the filters run.** Wait & Reply and
Wait & Call sit some four hundred lines above WSJT-X's filtering block, so a
check placed with the others would hide a station from the screen while the
radio went on calling it. The country check is made early, right after
`write_all`, and both blocks test it.

**`rcc` compresses the translation resource.** Do not verify a translation by
searching the built binary for Ukrainian text: past about 23 kB nothing is
visible that way, and no language is — Russian and Spanish included. Verify by
decompiling instead:

```sh
lconvert -i build/linux/wsjtx_uk.qm -o /tmp/back.ts
```

**cty.dat entity ids are line numbers.** They are handed out in parse order and
every cty.dat update renumbers them, so settings store the **primary prefix**
(`UR`, `JA`, `K`). A setting that remembered an id would quietly start naming a
different country.

**macOS builds are unsigned.** The signing step degrades gracefully when the
secrets are absent, so the `.pkg` is produced but Gatekeeper refuses it on
first run; the release notes tell operators to use Ctrl+click → Open.

---

## Translating

`translations/wsjtx_uk.ts`, currently about 1250 of 1650 strings. What is left
in English is deliberate: mode and band names, keyboard shortcuts, contest
names, units, on-air abbreviations. Qt renders an unfinished entry as the
original, so a partial translation never leaves the interface broken.

Extract new strings after adding any:

```sh
docker run --rm -v "$PWD:/src" -u "$(id -u):$(id -g)" -e HOME=/tmp -w /src \
  uaham/wsjtx-build lupdate -silent -recursive -I . \
  Configuration.cpp Configuration.ui main.cpp widgets logbook Network UaHam \
  models item_delegates validators -ts translations/wsjtx_uk.ts
```

Then use the tools in `.uaham/tools/`:

```sh
.uaham/tools/uk_list.py  translations/wsjtx_uk.ts MainWindow 1 40   # numbered dump
.uaham/tools/uk_apply.py translations/wsjtx_uk.ts MainWindow batch.txt
```

Each line of `batch.txt` is `index <TAB> source hash <TAB> translation`. **The
hash is not decoration.** Positional matching is what makes long HTML tooltips
workable, and its one failure mode is the list shifting under you — which
happened once and put ninety-one translations onto their neighbours, valid XML
and all. `uk_apply.py` now refuses to write unless every hash matches.

`uk_merge.py` and `uk_merge2.py` carry translations to the WSJT-Z fork: the
first matches full source text, the second matches sources differing only by
whitespace or Qt's rich-text wrapper. Both skip ambiguous matches rather than
guess.

---

## Boundaries

- **Never push this fork to `WSJTX/wsjtx`.** Upstream's release job ends with a
  `--force` push to its own master; those steps are removed here, and they
  must stay removed. Contributions go upstream as pull requests.
- **QRZ data is shown, never stored or forwarded.** QRZ's member agreement does
  not allow these details to be collected for anything but the contact being
  made. Nothing from `QrzLookup` may reach the log, a file, or the site.
- Do not add a dependency the platform workflows do not already install.
  `Qt5::WebSockets` was free because the TCI transceiver already needed it.
- Ask before changing the site protocol, the release tag format, or anything
  under `.github/workflows/` that touches signing or publishing.

---

See `.uaham/CHANGELOG.md` for what changed and why, and `.uaham/README.md` for
the operator-facing description.

Last updated: 2026-08-14
