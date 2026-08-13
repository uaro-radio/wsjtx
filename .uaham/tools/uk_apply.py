#!/usr/bin/env python3
"""Apply Ukrainian translations to a Qt .ts file by position, with a guard.

    uk_apply.py <file.ts> <context> <translations.txt>

Each line of the translations file is

    index <TAB> source marker <TAB> translation

in the order uk_list.py produced. The marker is the first characters of the
source and is verified before anything is written.

Matching by index rather than by full source text is deliberate: many sources
here are 900-character HTML tooltips with escaped entities and embedded
newlines, and retyping one to use as a dictionary key is a silent no-op when a
single character differs. But positional matching has its own failure: if
anything translates an entry between the dump and the apply, every later index
shifts by one and each translation lands on its neighbour — which is exactly
what happened once, and which reads as plausible until somebody opens the
dialog. Hence the marker, and hence nothing is written unless every marker
matches.

A translation for an HTML source may be written as plain text; it is wrapped
in the same rich-text skeleton, with "||" separating paragraphs. Literal "\\n"
becomes a newline.
"""
import re
import sys

sys.path.insert(0, __file__.rsplit("/", 1)[0])
from uk_list import collect, marker  # noqa: E402  - same filter, same order


def spans_of(ts_path, context):
    """Byte spans of every untranslated message, keyed by source text."""
    data = open(ts_path, encoding="utf-8").read()
    cm = re.search(r"<context>\s*<name>%s</name>(.*?)</context>" % re.escape(context),
                   data, re.S)
    block = cm.group(1)
    spans = {}
    for m in re.finditer(r"<message[^>]*>.*?</message>", block, re.S):
        chunk = m.group(0)
        if 'type="unfinished"' not in chunk:
            continue
        source = re.search(r"<source>(.*?)</source>", chunk, re.S)
        if not source:
            continue
        spans.setdefault(source.group(1), []).append((m.start(), m.end()))
    return data, cm, block, spans


def main(ts_path, context, translations_path):
    order = collect(ts_path, context)
    if order is None:
        print("контекст %r не знайдено" % context)
        return 1
    data, cm, block, spans = spans_of(ts_path, context)

    wanted, markers = {}, {}
    for line in open(translations_path, encoding="utf-8"):
        line = line.rstrip("\n")
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        parts = line.split("\t", 2)
        if len(parts) != 3:
            print("рядок без двох табуляцій, пропущено: %r" % line[:60])
            continue
        # Not named `marker`: that is the imported digest function, and
        # shadowing it here made the guard itself raise instead of check.
        index, expected_marker, translation = parts
        try:
            index = int(index)
        except ValueError:
            continue
        if translation.strip():
            wanted[index] = translation.replace("\\n", "\n")
            markers[index] = expected_marker

    # Verify every marker before writing anything: a half-applied batch is
    # worse than a rejected one.
    mismatches = []
    for index, expected in markers.items():
        if index > len(order):
            mismatches.append((index, expected, "<поза межами списку>"))
            continue
        actual = marker(order[index - 1])
        if actual != expected:
            mismatches.append((index, expected, actual))
    if mismatches:
        print("ВІДМОВА: нумерація не збігається з джерелами (%d розбіжностей)" % len(mismatches))
        for index, expected, actual in mismatches[:5]:
            print("  %d: очікував %s, у файлі %s" % (index, expected, actual))
        print("Файл не змінено. Перегенеруйте список і повторіть.")
        return 1

    edits = []
    for index, text in enumerate(order, start=1):
        if index not in wanted:
            continue
        translation = wanted[index]
        # A source wrapped in Qt's rich-text skeleton needs the translation
        # wrapped the same way, or the tooltip renders as literal markup.
        if text.startswith("&lt;html&gt;") and not translation.startswith("&lt;html&gt;"):
            paragraphs = "".join("&lt;p&gt;%s&lt;/p&gt;" % p.strip()
                                 for p in translation.split("||"))
            translation = ("&lt;html&gt;&lt;head/&gt;&lt;body&gt;%s&lt;/body&gt;&lt;/html&gt;"
                           % paragraphs)
        for start, end in spans.get(text, []):
            edits.append((start, end, translation))

    edits.sort(key=lambda e: e[0], reverse=True)   # right to left keeps offsets valid
    new_block = block
    for start, end, translation in edits:
        chunk = new_block[start:end]
        if 'numerus="yes"' in chunk:
            forms = "".join("<numerusform>%s</numerusform>" % translation for _ in range(3))
            replacement = re.sub(r"<translation[^>]*>.*?</translation>",
                                 "<translation>%s</translation>" % forms, chunk, flags=re.S)
        else:
            replacement = re.sub(r"<translation[^>]*>.*?</translation>",
                                 "<translation>%s</translation>" % translation,
                                 chunk, flags=re.S)
        new_block = new_block[:start] + replacement + new_block[end:]

    data = data[:cm.start(1)] + new_block + data[cm.end(1):]
    open(ts_path, "w", encoding="utf-8").write(data)
    print("контекст %s: перекладено %d записів (%d місць)" % (context, len(wanted), len(edits)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1], sys.argv[2], sys.argv[3]))
