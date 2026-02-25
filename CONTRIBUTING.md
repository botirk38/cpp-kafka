# Contributing to cpp-kafka

## Development Setup

- **Compiler**: GCC 13+ or Clang 15+ with C++23
- **CMake**: 3.28+
- **Build**: `cmake -B build -S . && cmake --build build`

## Code Standards

- Format with `clang-format` (see `.clang-format`)
- Run `clang-tidy` before submitting
- Add unit tests for new code in the module's `tests/` directory

## Testing

```bash
ctest --test-dir build --output-on-failure
```

Tests live in each module: `src/<module>/tests/`.

## Pull Requests

1. Ensure all tests pass
2. Run format: `find src -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i`
3. Keep changes focused and documented
