#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$project_root"
action="${1:-all}"
preset="${2:-portable-debug}"
for tool in cmake ninja ctest; do
    command -v "$tool" >/dev/null || { echo "Missing $tool; see BUILD.md" >&2; exit 1; }
done
export VCPKG_DISABLE_METRICS=1
case "$action" in
    configure) cmake --preset "$preset" ;;
    build) cmake --build --preset "$preset" --parallel 4 ;;
    test) ctest --preset "$preset" --output-on-failure ;;
    all)
        cmake --preset "$preset"
        cmake --build --preset "$preset" --parallel 4
        ctest --preset "$preset" --output-on-failure ;;
    run)
        shift 2
        target="${1:-cli}"
        if [[ $# -gt 0 ]]; then shift; fi
        case "$target" in desktop|server|relay|cli) ;; *) echo 'Unknown role' >&2; exit 2 ;; esac
        case "$preset" in *[!a-zA-Z0-9_-]*) echo 'Invalid preset' >&2; exit 2 ;; esac
        exec "out/build/$preset/bin/ghostwire-$target" "$@" ;;
    clean) cmake --build --preset "$preset" --target clean ;;
    diagnostics) cmake --version; ninja --version; ctest --version ;;
    *) echo 'Actions: configure build test all run clean diagnostics' >&2; exit 2 ;;
esac
