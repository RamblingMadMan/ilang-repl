# Infinity Lang REPL

Read.Eval.Print.Loop for the Infinity language.

## Building

For required dependencies see: [Infinity Lang Base Library](https://github.com/RamblingMadMan/ilang-base)

From the source directory run the following commands:

```bash
git submodule update --init --recursive
mkdir build && cd build
cmake ..
cmake --build .
```

## Running

After following the building steps, run `build/ilang-repl`

You should be met with a prompt like this:

```bash
>
```

Currently basic maths expressions, string expressions, list literals, product literals and `exit` or `quit` are accepted.
