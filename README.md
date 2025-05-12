# 🚀 AOI Editor - Lightning-Fast Competitive Programming Editor

![AOI Editor Screenshot](docs/screenshot.png)

A minimalist, terminal-based code editor optimized for competitive programming contests. Built with C++ and ncurses for maximum performance during time-sensitive competitions.

## ✨ Features

- **Blazing Fast** - No GUI overhead, pure terminal power
- **Smart C++ Highlighting** - 50+ keywords with 6 color groups
- **One-Key Compilation** - F5 to compile & run
- **Zero Config** - Works out of the box
- **<1MB Memory** - Lightweight even on old machines

## ⚡ Quick Start

```bash
# Clone & Build
git clone https://github.com/mohamedboukerche22/AOI-IDE.git
cd AOI-IDE
make

# Run with:
./bin/aoi solution.cpp
```
```cpp
// Special optimizations for CP
#include <bits/stdc++.h>  // ← Detected and highlighted
using namespace std;

int main() {
    ios_base::sync_with_stdio(false);  // ← Competitive pattern recognized
    cin.tie(nullptr);
    
    // Fast I/O template
    int n;
    cin >> n;
    cout << n*2 << '\n';  // ← '\n' highlighted over endl
}
```

### 2. INSTALL.md (Installation Guide)

````markdown
# AOI Editor Installation

## Linux/macOS

### Dependencies
```bash
# Ubuntu/Debian
sudo apt install g++ libncurses-dev

# Fedora
sudo dnf install gcc-c++ ncurses-devel

# macOS (Homebrew)
brew install ncurses
````


### install
```
g++ aoi.cpp -o aoi -lncurses
```
### run
```
./aoi {yourcodename}.cpp
```

```graph TD

    A[Core C++ Engine] --> B[Platform-Specific Adapters]
    B --> C[Linux/ncurses]
    B --> D[Windows/PDCurses]
    B --> E[Web/Emscripten]
    B --> F[Mobile/Termux]
```
