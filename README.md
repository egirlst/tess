# Tess Language

The programming language that connects the web with the system.

- Native HTTP built-in, no extra libraries
- Direct file and memory control when needed
- Clean, expressive syntax

## Quick Start

- Run a file: `tess run main.tess` (alias: `ts run main.tess`)
- Start a project: `tess new myproj` then `tess run main.tess`
- Interactive shell: `tess repl`

## Example 1 — Hello

```tess
$$ Minimal hello program
f! main {
    print:: "Hello"
}

start >.<
```

## Example 2 — Web Request

```tess
$$ Built-in HTTP GET
f! main {
    response = request:: "GET" "https://httpbin.org/get"
    print:: response
}

start >.<
```

## Example 3 — File I/O + Memory

```tess
$$ Write and read a file, then free memory
f! main {
    $$ Allocate memory
    ptr = mem.alloc(128)

    $$ Write to a file
    file = f.open("example.txt", "w")
    file.write("Tess writes files easily")
    file.close()

    $$ Read the file back
    file = f.open("example.txt", "r")
    content = file.read()
    file.close()
    print:: content

    $$ Free memory
    mem.free(ptr)
}

start >.<
```

## Notes

- Comments use `$$` at the start of a line.
- Entry point is `f! main { ... }` followed by `start >.<`.
