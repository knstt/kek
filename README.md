# Kek

Kek is a small systems language design that generates C code.

The name stands for nothing. That is the joke.

Source files use the `.kek` extension.

Build the lexer:

```sh
make
```

Print tokens for a source file:

```sh
build/keklex example/main.kek
```

Run the lexer tests:

```sh
make test
```

Examples:

- `example/main.kek` current language sketch
- `example/future.kek` ideas for later
- `example/docs.kek` documentation comment style
