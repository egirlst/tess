# Tess Language

**The programming language that connects the web with the system.**

Tess is a new, lightweight programming language designed for modern developers who need both high-level web capabilities and low-level system control. It combines a unique, expressive syntax with powerful built-in tools.

## Why Tess?

*   **🌐 Native Web Support**: HTTP requests are built right into the language core. No extra libraries needed.
*   **🔧 Low-Level Power**: Direct memory management and file control when you need it.
*   **⚡ Fast & Efficient**: Built on a custom C-based interpreter.
*   **🎨 Unique Syntax**: Expressive and readable code structure.

## A Taste of Tess

### 1. Simple & Clean
Writing code in Tess is straightforward. Here is your standard entry point:

```tess
f! main {
    print:: "Hello, World!"
}

start >.<
```

### 2. Built for the Web
Forget installing heavy packages just to make an API call. In Tess, it's one line:

```tess
f! main {
    $$ Fetch data directly
    response = request:: "GET" "https://api.example.com/data"
    print:: response
}

start >.<
```

### 3. System Control
Need to manage memory manually? Tess gives you the keys to the kingdom:

```tess
f! main {
    $$ Direct memory allocation
    ptr = mem.alloc(1024)
    
    $$ Write to file
    file = f.open("log.txt", "w")
    file.write("System active")
    file.close()
    
    mem.free(ptr)
}

start >.<
```

## Get Started

Download the latest compiler and start building today.
