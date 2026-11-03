# Unix Shell Interpreter (fsh)

## About

**FSH** is a custom Unix shell interpreter implemented from scratch in C as a third-year Bachelor's (L3) project at Université Paris Cité. Designed to explore low-level systems programming, the project replicates the core mechanics of a Unix shell, handling command line parsing, execution of system binaries, built-in commands, and control flow structures.

*Note: As an academic project focused on core shell mechanics, certain advanced features like complex pipelines and signal management have partial limitations. The source code and comments may contain some French notes.*

## Tech Stack

- **Language:** C
- **System Programming:** POSIX APIs, process management (`fork`, `exec`), file descriptors, and system calls.
- **Build System:** Makefile

## Key Features

- **Command Execution:** Parses user input to execute external system binaries and handle arguments.
- **Built-in Commands:** Native implementation of essential commands including `cd`, `pwd`, `exit`, and `ftype`.
- **Control Structures:** Support for conditional blocks (`if / else`) and iterative loops (`for`).
- **I/O Redirections:** Management of standard input/output redirections (`>`, `<`, etc.).
- **Robust Error Handling:** Comprehensive checking and reporting for system calls, syntax errors, and invalid commands to prevent unexpected crashes.

## Known Limitations & Academic Scope

As part of the project's academic scope, certain features are partially implemented or intentionally left out:
- **Pipelines:** Complex multi-command piping (`|`) has limited behavior so unexpected behavior and crash can happen.
- **Signals:** Signal handling has limitations and is partially implemented.
- **Parallel Execution:** Parallel loops are partially implemented.

## Project Structure

```text
📁 fsh/
│
├── 📁 docs/
│   ├── ARCHITECTURE.md                 # Project architecture
│   └── AUTHORS.md                      # Team of the project
│
├── 📁 src/
│   ├── cd.c / cd.h                     # Built-in 'cd' command implementation
│   ├── cmd.c / cmd.h                   # Command parsing and routing
│   ├── cmdutils.c / cmdutils.h         # Core utility functions (string parsing & blocks)
│   ├── externalcmd.c / externalcmd.h   # External binary execution
│   ├── for.c / for.h                   # Loop structures
│   ├── fsh.c / fsh.h                   # Main shell loop and entry point
│   ├── ftype.c / ftype.h               # Built-in 'ftype' command implementation
│   ├── if.c / if.h                     # Control flow (if/else logic)
│   ├── pwd.c / pwd.h                   # Built-in 'pwd' command implementation
│   └── redirection.c / redirection.h   # Input/Output redirection handling
│
├── 📄 LICENSE                          # License of the project
├── 📄 Makefile                         # Build configuration
└── 📄 README.md                        # Project documentation
```

## Getting Started

### Prerequisites

* A Unix-like environment (Linux, WSL, or macOS)
* GCC compiler and make

## Compilation & Execution

1. Clone the repository:
```bash
git clone https://github.com/SuyuKuka/c-unix-shell fsh
cd fsh
```

2. Compile the project:
```bash
make
```

3. Run the shell:
```bash
./fsh
```
*(Alternative shortcuts: make c to compile and run directly)*

## Team

This project was developed collaboratively as a 3-person team:

- **CHEN Michel** - Systems Programming & Shell Architecture
  [![GitHub](https://img.shields.io/badge/GitHub-Profile-181717?logo=github)](https://github.com/SuyuKuka)
  [![LinkedIn](https://img.shields.io/badge/LinkedIn-Profile-0A66C2?logo=linkedin)](https://linkedin.com/in/chen-michel)

- **HUYNH Minh Van Claude** - Systems Programming & Shell Architecture
  [![GitHub](https://img.shields.io/badge/GitHub-Profile-181717?logo=github)](https://github.com/huynh-claude)

- **ROIG Lucas** - Systems Programming & Shell Architecture
  [![GitHub](https://img.shields.io/badge/GitHub-Profile-181717?logo=github)](https://github.com/ll-tykas)

## License

This project is licensed under the GPL 3.0 License — see [LICENSE](LICENSE) for details.