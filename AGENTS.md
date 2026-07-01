# AGENTS.md

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

**Never `rm -rf build`** — CPM downloads and caches dependencies inside the build directory. Cleaning it forces re-downloading all deps.

## Test

```bash
ctest --test-dir build                  # all tests
ctest --test-dir build -R <test-name>   # single test (use hyphens: test-cmrc-file-interface)
```

Test executables are auto-generated from `test/**/*.cpp`. Filename underscores become hyphens in the test name (e.g. `test_cmrc_file_interface.cpp` → `test-cmrc-file-interface`).

## Format & Lint

- `clang-format -i` — uses `.clang-format` (LLVM base, 4-space indent)
- `-Wall -Wextra -Werror` on library targets — treat warnings as errors

## Code Style

- **C++26** standard required. clangd config in `.clangd` strips module flags.
- `snake_case` for namespaces, classes, functions, variables
- `PascalCase` only for RmlUi virtual method overrides (`Open`, `Close`, `Read`, `Seek`, `Tell`)
- **Brace initialization** preferred: `auto x{value}`, not `auto x = value`
- **SPDX license header** required on every source file (MIT)
- `#pragma once` for header guards
- `noexcept` on most method implementations
- Macros prefixed with `ZEPHYRUS_`

## Architecture

- **CRTP managers**: `abstract::manager<Derived>` base class — new subsystems inherit it and implement `init_impl()` returning `std::expected<void, std::string>`
- **Error handling**: `std::expected` (C++23), not exceptions
- **Dependency injection**: Boost.DI (`boost/di.hpp`), wired in `global_zephyrus()`
- **Asset embedding**: CMRC — add resources in `asset/CMakeLists.txt`, declare with `CMRC_DECLARE` in headers
- **Logging**: spdlog with custom `ZEPHYRUS_*` macros (`ZEPHYRUS_TRACE`, `ZEPHYRUS_INFO`, etc.)

## Build Targets

| Target | Type | Description |
|---|---|---|
| `zephyrus-core` | Static lib | Core library (`src/zephyrus/`) |
| `zephyrus-standalone` | Executable | Standalone entry point |
| `zephyrus-acr` | Shared lib | ACR entry point (Linux `__attribute__((constructor))`) |

## Gotchas

- CMake 4.2+ required
- Linux: static-links libstdc++ and libgcc; release builds use `-fvisibility=hidden`
- `NDEBUG` controls spdlog level: `trace` in debug, `info` in release
- Dependencies: CPM-managed (spdlog, fmt, Boost.UT, Boost.DI, RmlUi, freetype, zlib, bzip2, brotli, libpng, harfbuzz)
- All deps are fetched at configure time; no git submodules
