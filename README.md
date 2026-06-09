# Kek

Kek is a small systems language design that generates C code.

The name stands for nothing. That is the joke.

Source files use the `.kek` extension.

Build:

```sh
cc -std=c11 -Wall -Wextra -pedantic main.c source.c tokenizer.c ast.c ast_json.c codegen_c.c -o kek
```

Run the fixed smoke compile:

```sh
./kek
```

This reads `tmp.kek`, writes generated C to `out/out.c`, writes AST JSON
to `out/ast.json`, and writes a typed frontend summary to `out/module.txt`.

Run the smoke test:

```sh
make test
```

Open `ast_viewer.html` in a browser and load `ast.json` to inspect the tree.

The AST is currently structural rather than semantic: file, statement, block,
parenthesized group, bracket group, and token nodes. It is built from the
existing tokenizer token stream.

Current bootstrap state: the typed frontend builds semantic declarations,
fields, enum variants, statements, Pratt-parsed expressions, and types.
Semantic analysis and active C generation consume the typed AST. The structural
AST is still emitted as JSON for debugging and remains in the C compiler as a
bootstrap parser substrate.

Examples:

- `example/main.kek` current language sketch
- `example/future.kek` ideas for later
- `example/docs.kek` documentation comment style

## Self-Hosting Checklist

Kek will keep using C interop for file IO, allocation, libc string/memory
functions, and process exit during the bootstrap. A full standard library is
not required for the first self-host.

- [x] Fixed no-argument smoke compile from `tmp.kek` to `out/out.c`
- [x] Smoke test that builds and runs generated C
- [x] Whitespace-agnostic smoke test for minified Kek syntax
- [x] Raw `extern "C"` blocks for bootstrap C interop
- [x] Directory import smoke path with package-style C symbol prefixes
- [x] Base C backend coverage for structs, functions, methods, default args,
      named args, aliases, enums, unions, and switches
- [x] Typed frontend declaration model alongside the structural AST
- [x] Typed parser for top-level base declarations
- [x] Typed frontend coverage summary for declarations, statements,
      expressions, and types
- [x] Unknown top-level declarations fail typed parsing
- [x] Smoke coverage for base control flow: `if`, `else`, `for`, `while`,
      `do while`, `switch`, `break`, and `continue`
- [x] Smoke coverage for explicit casts and fixed arrays
- [x] Smoke coverage for C allocation interop through `extern "C"`
- [x] Global symbol collection scaffold for typed modules
- [x] Typed frontend AST nodes for declarations, params, statements,
      expressions, and types
- [x] Expression parser with precedence and assignment handling
- [x] Statement parser for blocks, declarations, `if`, loops, `switch`,
      `break`, `continue`, and `return`
- [x] Type parser for builtins, pointers, fixed arrays, user types, and aliases
- [x] Symbol tables for modules, globals, types, functions, and local scopes
- [x] Semantic checks for names, calls, assignments, returns, and control flow
- [x] C backend generated from typed AST instead of structural token patterns
- [x] C interop declarations for allocation, file IO, string/memory helpers,
      formatted output, and exit
- [ ] Port tokenizer to Kek
- [ ] Port typed parser to Kek
- [ ] Port semantic analysis to Kek
- [ ] Port C backend to Kek
- [ ] Stage 1 Kek compiler builds the same smoke program as the C compiler
- [ ] Stage 2 Kek compiler rebuilds itself and passes the smoke test
