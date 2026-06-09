#!/usr/bin/env python3
import sys


def needs_space(left, right):
    if not left or not right:
        return False
    return (left.isalnum() or left == "_") and (right.isalnum() or right == "_")


def find_matching_brace(text, open_index):
    depth = 0
    i = open_index
    in_string = False
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
            if c == "\\":
                i += 2
                continue
            if c == '"':
                in_string = False
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
            in_string = True
            i += 1
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


def minify_kek(text):
    out = []
    i = 0
    in_string = False
    in_line_comment = False
    in_block_comment = False
    while i < len(text):
        if text.startswith('extern "C"', i):
            brace = text.find("{", i)
            if brace >= 0:
                end = find_matching_brace(text, brace)
                if end >= 0:
                    segment = text[i:end + 1]
                    if out and needs_space(out[-1], segment[0]):
                        out.append(" ")
                    out.append(segment)
                    i = end + 1
                    continue

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
        if c == "/" and n == "/":
            in_line_comment = True
            i += 2
            continue
        if c == "/" and n == "*":
            in_block_comment = True
            i += 2
            continue
        if c == '"':
            if out and needs_space(out[-1], c):
                out.append(" ")
            out.append(c)
            in_string = True
            i += 1
            continue
        if c.isspace():
            j = i + 1
            while j < len(text) and text[j].isspace():
                j += 1
            prev = out[-1] if out else ""
            nxt = text[j] if j < len(text) else ""
            if needs_space(prev, nxt):
                out.append(" ")
            i = j
            continue
        out.append(c)
        i += 1
    return "".join(out)


if __name__ == "__main__":
    sys.stdout.write(minify_kek(sys.stdin.read()))
