# Tess Programming Language

**Tess** is a versatile programming language compiler and interpreter designed for efficiency and ease of use. It comes with a robust set of built-in tools for package management, code formatting, linting, and testing.

## Features

- **Compiler & Interpreter**: Run scripts directly or build them for distribution.
- **Package Management**: Built-in package manager to install, update, and remove dependencies.
- **Developer Tooling**: Includes a formatter (`fmt`), linter (`lint`), and test runner (`test`) out of the box.
- **Interactive Shell**: a REPL for quick experimentation.
- **Virtual Environments**: Manage isolated environments with `venv`.
- **Project Scaffolding**: Quickly start new projects with `new`.
- **Lenient syntax model**: Tess intentionally avoids strict syntax and formatting rules. Bracket placement and code style are not enforced, provided all logic is correctly scoped within << >> or { } blocks per function or class.
## Build & Installation

### Prerequisites

- GCC Compiler
- Make

## Installation

You can download the latest pre-built binaries for Windows, macOS, and Linux from the [Releases Page](https://github.com/egirlst/tess/releases/).

### Building from Source

To build Tess from source, run the following command in the root directory:

```bash
make
```

This will compile the source code and generate the binaries in the `bin/` directory:
- `bin/tess` (Main executable)
- `bin/ts` (Alias)

## Usage

You can use either `tess` or the short alias `ts` to run commands.

### Running Code

```bash
# Run a script
tess run script.tess

# Run inline code
tess exec "print('Hello, World!')"

# Start the REPL
tess repl
```

### Project Management

```bash
# Create a new project
tess new my_project

# Build a project
tess build main.tess
```

### Package Management

```bash
# Install a package
tess install package_name

# Install dependencies from .tess.noah (local config)
tess i .

# Uninstall a package
tess uninstall package_name
```

### Development Tools

```bash
# Format code
tess fmt file.tess

# Lint code
tess lint file.tess

# Check syntax
tess check file.tess

# Run tests
tess test
```

## License

[Add License Information Here]
