#!/usr/bin/env python3
"""Dump the untranslated messages of one context, numbered for uk_apply.py.

    uk_list.py <file.ts> <context> [from] [to]

Each line is  index <TAB> marker <TAB> full source, where the marker is the
first characters of the source. uk_apply.py checks the marker before writing,
so a list that has gone stale — anything else translated an entry in between,
shifting the numbering — is refused instead of silently applied one row off.

The filter and the order here are the contract with uk_apply.py. Change one
and you must change both.
"""
import hashlib
import re
import sys


def marker(text):
    """Short digest of the source. A prefix would not do: every HTML tooltip
    here begins with the same rich-text preamble, so prefixes collide and the
    guard would wave through exactly the shift it exists to catch."""
    return hashlib.sha1(text.encode("utf-8")).hexdigest()[:8]


def collect(ts_path, context):
    data = open(ts_path, encoding="utf-8").read()
    cm = re.search(r"<context>\s*<name>%s</name>(.*?)</context>" % re.escape(context),
                   data, re.S)
    if not cm:
        return None
    order, seen = [], set()
    for m in re.finditer(r"<message[^>]*>.*?</message>", cm.group(1), re.S):
        chunk = m.group(0)
        if 'type="unfinished"' not in chunk:
            continue
        source = re.search(r"<source>(.*?)</source>", chunk, re.S)
        if not source:
            continue
        text = source.group(1)
        # Only the truly empty are skipped now. The first passes excluded
        # anything short and alphanumeric as "technical", which also threw out
        # Grid, Gain, Dialog, Contest: and New DXCC — words that are as
        # user-facing as any sentence. Deciding what is technical is a job for
        # the translator, not for a regular expression.
        if len(text) < 2:
            continue
        if text in seen:
            continue
        seen.add(text)
        order.append(text)
    return order


def main(ts_path, context, first=1, last=10 ** 9):
    order = collect(ts_path, context)
    if order is None:
        print("контекст %r не знайдено" % context)
        return 1
    print("# %s: %d untranslated" % (context, len(order)))
    for index, text in enumerate(order, start=1):
        if first <= index <= last:
            flat = text.replace("\n", "\\n")
            print("%d\t%s\t%s" % (index, marker(text), flat))
    return 0


if __name__ == "__main__":
    args = sys.argv[1:]
    sys.exit(main(args[0], args[1],
                  int(args[2]) if len(args) > 2 else 1,
                  int(args[3]) if len(args) > 3 else 10 ** 9))
