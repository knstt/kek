# Kek

Kek is a small systems language design that generates C code.

The name stands for nothing. That is the joke.

Source files use the `.kek` extension.

Build:

```sh
cc -std=c11 -Wall -Wextra -pedantic main.c source.c tokenizer.c ast.c ast_json.c codegen_c.c -o kek
```

Print tokens for a source file:

```sh
./kek tmp.kek
```

Print the tokenizer-backed AST:

```sh
./kek --ast tmp.kek
```

Write AST JSON:

```sh
./kek --ast-json tmp.kek ast.json
```

Generate C:

```sh
./kek --c tmp.kek
./kek --c tmp.kek tmp.c
```

Open `ast_viewer.html` in a browser and load `ast.json` to inspect the tree.

The AST is currently structural rather than semantic: file, statement, block,
parenthesized group, bracket group, and token nodes. It is built from the
existing tokenizer token stream.

Examples:

- `example/main.kek` current language sketch
- `example/future.kek` ideas for later
- `example/docs.kek` documentation comment style
