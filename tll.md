# Tess Language Specification (TLL)

> **Note to AI Agents & LLMs:**
> This document defines the **Tess Programming Language**, a custom language with unique syntax for functions (`f!`), classes (`cls`), and system operations. When generating code for Tess, strictly follow the grammar and examples provided below. Do not assume syntax from Python, C, or Rust unless explicitly shared.

---

## 1. Syntax Overview

### Lenient parsing: 
Tess does not enforce strict formatting or bracket placement. Parsers should rely on block delimiters ({} or << >>) rather than whitespace or line structure.

### Comments
- **Single-line**: `# This is a comment`
- **Multi-line**: `$$ This is a multi-line comment $$`

### Identifiers
- Alphanumeric + `_`.
- Can contain `:` (e.g., `print::`).

### Blocks
- Standard blocks: `{ ... }`
- Inner blocks (alternative): `<< ... >>`

### Entry Point
Every program can define an entry point using the `start` keyword:
```tess
start >main<
```
Only one `start >func<` entry point is allowed per program.

---

## 2. Keywords & Core Structure

| Keyword | Usage | Example |
| :--- | :--- | :--- |
| `f!` | Function definition | `f! my_func(arg1) { ... }` |
| `cls` | Class definition | `cls MyClass { ... }` |
| `ret` | Return value | `ret 0` |
| `var` / `name =` | Variable assignment | `x = 10` |
| `if` / `else` | Conditionals | `if x > 0 { ... } else { ... }` |
| `while` | Loop | `while x < 10 { ... }` |
| `print::` | Print to stdout | `print:: "Hello"` |
| `request::` | HTTP Request | `request:: "GET" "http://api.com"` |
| `new` | Instantiate Object | `x = new MyClass()` |
| `add` | Import module | `add my_module` |

---

## 3. Grammar (Simplified EBNF)

```ebnf
program         ::= statement*
statement       ::= function_def | class_def | assignment | if_stmt | while_stmt | 
                    print_stmt | return_stmt | import_stmt | start_stmt | expr_stmt

function_def    ::= "f!" identifier "(" param_list? ")" block
class_def       ::= "cls" identifier "{" class_member* "}"
assignment      ::= identifier "=" expression
if_stmt         ::= "if" expression block ("else" block)?
while_stmt      ::= "while" expression block

print_stmt      ::= "print::" expression ("," expression)*
return_stmt     ::= "ret" expression
import_stmt     ::= "add" identifier ("from" identifier)? ("as" identifier)?
start_stmt      ::= "start" ">" identifier "<"

block           ::= "{" statement* "}" | "<<" statement* ">>"
expression      ::= literal | identifier | binary_op | function_call | member_access
```

---

## 4. Examples

### Hello World
```tess
f! main() {
    print:: "Hello, World!"
}

start >main<
```

### Variables & Data Types
```tess
f! main {
    # Numbers
    x = 42
    y = 3.14
    
    # Strings
    name = "Tess"
    
    # Lists
    items = [1, 2, 3]
    
    # Dicts/Objects are implicit in some contexts or via built-ins
    
    print:: "Count: ", x
}
```

### Control Flow
```tess
f! logic(val) {
    if val > 10 {
        print:: "Value is large"
    } else {
        print:: "Value is small"
    }
    
    i = 0
    while i < 5 {
        print:: i
        i = i + 1
    }
}
```

### Functions
```tess
f! add(a, b) {
    ret a + b
}

f! main() {
    result = add(10, 20)
    print:: "Result: ", result
}
```

### Classes & Objects
```tess
cls Dog {
    f! bark() {
        print:: "Woof!"
    }
}

f! main() {
    d = new Dog()
    d.bark()
}
```

### Imports
```tess
# Import a standard module
add math

# Import from a package
add utils from my_pkg

# Import with alias
add network as net
```

---

## 5. Built-in Standard Library

### Global Functions
- `len(obj)`: Get length of string/list.
- `read_file(path)`: Read file content.
- `write_file(path, content)`: Write to file.
- `sqrt(n)`, `abs(n)`, `max(a, b)`, `min(a, b)`: Math helpers.
- `clock()`: Get current time.

### System Objects
Tess provides global objects for system interaction:

#### File I/O (`f`)
```tess
file = f.open("test.txt", "r")
# Operations: read, write, close
```

#### System (`sys`)
```tess
sys.sleep(1000)  # Sleep ms
sys.exit(0)      # Exit program
```

#### Memory (`mem`) - Advanced
```tess
ptr = mem.alloc(1024)
mem.set(ptr, 255)
val = mem.get(ptr)
mem.free(ptr)
```

#### Assembly/JIT (`asm`) - Advanced
```tess
# Allocate executable memory
code = asm.alloc_exec(64)
# Write machine code...
asm.exec(code)
```

---

## 6. HTTP & Networking
Tess has first-class support for HTTP requests.

```tess
f! fetch_data() {
    # Simple GET
    resp = request:: "GET" "https://tess.sh/test/data"
    print:: resp
    
    # Or using http:: alias
    http:: "POST" "https://tess.sh/test/submit" "payload"
}
```