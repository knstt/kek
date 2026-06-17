# Kek

Kek is a small systems language compiler that generates C.

The current executable contract is intentionally narrow:

- `tmp.kek` is the main smoke program.
- `std/*.kek` is the bootstrap standard library surface.
- `fmt.kek` is the larger Kek-written tool smoke test.
- `tests/fixtures/` contains compiler/API fixtures.

Build the compiler:

```sh
make build
```

Run the full smoke and API test suite:

```sh
make test
```

Compile a Kek source file:

```sh
bin/kek build tmp.kek
```

By default this writes generated C to `out/out.c`, structural AST JSON to
`out/ast.json`, and a typed frontend summary to `out/module.txt`.

Custom output paths:

```sh
bin/kek build tmp.kek -o out/custom.c --ast-json out/custom.json --summary out/custom.txt
```

Use `--out-dir <dir>` to change the directory for default output names.
