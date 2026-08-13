#!/usr/bin/env python3
"""Carry finished translations from one .ts file into another.

    uk_merge.py <from.ts> <into.ts>

Matches on the full source text, preferring the same context and falling back
to a different one only when every context that has this source agrees on the
translation. Matching on text rather than position is right here — the two
trees are forks of the same program and share most of their strings verbatim —
and an ambiguous fallback is skipped rather than guessed.

Only entries still marked unfinished in the destination are touched.
"""
import re
import sys


def harvest(path):
    """(context, source) -> translation, for finished entries only."""
    data = open(path, encoding="utf-8").read()
    by_pair, by_source = {}, {}
    for cm in re.finditer(r"<context>\s*<name>([^<]+)</name>(.*?)</context>", data, re.S):
        context = cm.group(1)
        for mm in re.finditer(r"<message[^>]*>.*?</message>", cm.group(2), re.S):
            chunk = mm.group(0)
            if 'type="unfinished"' in chunk:
                continue
            source = re.search(r"<source>(.*?)</source>", chunk, re.S)
            translation = re.search(r"<translation[^>]*>(.*?)</translation>", chunk, re.S)
            if not source or not translation:
                continue
            text, value = source.group(1), translation.group(1)
            if not value.strip():
                continue
            by_pair[(context, text)] = value
            by_source.setdefault(text, set()).add(value)
    # Only sources whose translation is the same wherever they appear are
    # usable without their context.
    unambiguous = {t: next(iter(v)) for t, v in by_source.items() if len(v) == 1}
    return by_pair, unambiguous


def main(src_path, dst_path):
    by_pair, unambiguous = harvest(src_path)
    data = open(dst_path, encoding="utf-8").read()
    filled = {"same context": 0, "other context": 0}
    skipped = 0

    def do_context(cm):
        context = cm.group(1)

        def fill(mm):
            nonlocal skipped
            chunk = mm.group(0)
            if 'type="unfinished"' not in chunk:
                return chunk
            source = re.search(r"<source>(.*?)</source>", chunk, re.S)
            if not source:
                return chunk
            text = source.group(1)
            value = by_pair.get((context, text))
            which = "same context"
            if value is None:
                value = unambiguous.get(text)
                which = "other context"
            if value is None:
                skipped += 1
                return chunk
            filled[which] += 1
            if 'numerus="yes"' in chunk:
                if "<numerusform>" not in value:
                    value = "".join("<numerusform>%s</numerusform>" % value for _ in range(3))
                return re.sub(r"<translation[^>]*>.*?</translation>",
                              "<translation>%s</translation>" % value, chunk, flags=re.S)
            return re.sub(r"<translation[^>]*>.*?</translation>",
                          "<translation>%s</translation>" % value, chunk, flags=re.S)

        body = re.sub(r"<message[^>]*>.*?</message>", fill, cm.group(2), flags=re.S)
        return "<context>\n    <name>%s</name>%s</context>" % (context, body)

    data = re.sub(r"<context>\s*<name>([^<]+)</name>(.*?)</context>", do_context, data, flags=re.S)
    open(dst_path, "w", encoding="utf-8").write(data)
    print("перенесено: %d (той самий контекст) + %d (інший контекст) = %d"
          % (filled["same context"], filled["other context"], sum(filled.values())))
    print("лишилося без перекладу: %d" % skipped)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
