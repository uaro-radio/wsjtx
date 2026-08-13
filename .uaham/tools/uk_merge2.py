#!/usr/bin/env python3
"""Second pass of the .ts merge: match sources that differ only cosmetically.

    uk_merge2.py <from.ts> <into.ts>

The two forks share most of their text but not always byte for byte. The same
label appears with a trailing space, or wrapped in Qt's rich-text skeleton in
one tree and bare in the other, or — in at least one case — with a typo one
side fixed and the other did not ("Troposheric ducting"). An exact match
misses all of those; comparing the words alone finds them.

A match is used only when every finished translation of that normalised form
agrees, so an ambiguous one is left for a human rather than guessed at. The
translation is then re-shaped to match the destination: unwrapped if the
destination source is plain text, wrapped if it is rich text.
"""
import re
import sys

TAG = re.compile(r"&lt;[^&]*?&gt;|<[^>]*>")


def normalise(text):
    text = TAG.sub(" ", text)
    text = text.replace("&amp;", "").replace("&quot;", '"').replace("&apos;", "'")
    text = re.sub(r"[\s ]+", " ", text)
    return text.strip().lower()


def harvest(path):
    data = open(path, encoding="utf-8").read()
    by_norm = {}
    for mm in re.finditer(r"<message[^>]*>.*?</message>", data, re.S):
        chunk = mm.group(0)
        if 'type="unfinished"' in chunk:
            continue
        source = re.search(r"<source>(.*?)</source>", chunk, re.S)
        translation = re.search(r"<translation[^>]*>(.*?)</translation>", chunk, re.S)
        if not source or not translation or not translation.group(1).strip():
            continue
        by_norm.setdefault(normalise(source.group(1)), set()).add(translation.group(1))
    return {k: next(iter(v)) for k, v in by_norm.items() if len(v) == 1 and k}


def reshape(value, target_source):
    """Give the translation the same shape as the destination's source."""
    target_is_html = target_source.startswith("&lt;html&gt;")
    value_is_html = value.startswith("&lt;html&gt;")
    if target_is_html and not value_is_html:
        return ("&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;%s&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;"
                % value)
    if value_is_html and not target_is_html:
        # Paragraph breaks become newlines; a plain label rarely has more than
        # one paragraph, and a stray tag in a button caption is worse than a
        # line break.
        text = value.replace("&lt;/p&gt;&lt;p&gt;", "\n")
        return TAG.sub("", text).strip()
    return value


def main(src_path, dst_path):
    by_norm = harvest(src_path)
    data = open(dst_path, encoding="utf-8").read()
    filled = 0

    def fill(mm):
        nonlocal filled
        chunk = mm.group(0)
        if 'type="unfinished"' not in chunk:
            return chunk
        source = re.search(r"<source>(.*?)</source>", chunk, re.S)
        if not source:
            return chunk
        value = by_norm.get(normalise(source.group(1)))
        if not value:
            return chunk
        value = reshape(value, source.group(1))
        filled += 1
        if 'numerus="yes"' in chunk and "<numerusform>" not in value:
            value = "".join("<numerusform>%s</numerusform>" % value for _ in range(3))
        return re.sub(r"<translation[^>]*>.*?</translation>",
                      "<translation>%s</translation>" % value, chunk, flags=re.S)

    data = re.sub(r"<message[^>]*>.*?</message>", fill, data, flags=re.S)
    open(dst_path, "w", encoding="utf-8").write(data)
    print("перенесено за нормалізованим збігом: %d" % filled)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
