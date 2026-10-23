#!/usr/bin/env bash
# No firmware, audio host, graphics libraries or patched Unicorn required.
# Usage: DPF_ROOT=/path/to/DPF vst/tests/run-input-tests.sh
set -euo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
: "${DPF_ROOT:?Set DPF_ROOT to the DPF source checkout used by the plugin}"
build=$(mktemp -d)
trap 'rm -rf "$build"' EXIT
python3 "$root/scripts/gen-layout.py" "$root/../core/layout/digitone2_panel.json" "$build/panel_layout.hpp"
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror ${CXXFLAGS:-} \
    -I"$DPF_ROOT/dgl" -I"$root" -I"$build" "$root/tests/input.cpp" -o "$build/input-tests"
"$build/input-tests"
