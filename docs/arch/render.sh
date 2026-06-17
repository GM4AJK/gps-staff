#!/bin/sh
# Render all PlantUML diagrams to PNG for GitHub display.
# Run from the repo root or from docs/arch/:
#   docs/arch/render.sh
# Requires: plantuml, graphviz (sudo apt install plantuml graphviz)
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
plantuml -tpng "$SCRIPT_DIR/img/"*.puml
echo "Done — $(ls "$SCRIPT_DIR/img/"*.png | wc -l) PNGs written to docs/arch/img/"
