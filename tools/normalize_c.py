#!/usr/bin/env python3
import sys


def normalize_c(text):
    out = []
    i = 0
    in_string = False
    in_char = False
    in_line_comment = False
    in_block_comment = False
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if in_line_comment:
            if c == "\n":
                in_line_comment = False
            i += 1
            continue
        if in_block_comment:
            if c == "*" and n == "/":
                in_block_comment = False
                i += 2
            else:
                i += 1
            continue
        if in_string:
            out.append(c)
            if c == "\\":
                if i + 1 < len(text):
                    out.append(text[i + 1])
                i += 2
                continue
            if c == '"':
                in_string = False
            i += 1
            continue
        if in_char:
            out.append(c)
            if c == "\\":
                if i + 1 < len(text):
                    out.append(text[i + 1])
                i += 2
                continue
            if c == "'":
                in_char = False
            i += 1
            continue
        if c == "/" and n == "/":
            in_line_comment = True
            i += 2
            continue
        if c == "/" and n == "*":
            in_block_comment = True
            i += 2
            continue
        if c == '"':
            out.append(c)
            in_string = True
            i += 1
            continue
        if c == "'":
            out.append(c)
            in_char = True
            i += 1
            continue
        if c.isspace():
            i += 1
            continue
        out.append(c)
        i += 1
    return "".join(out)


if __name__ == "__main__":
    sys.stdout.write(normalize_c(sys.stdin.read()))
