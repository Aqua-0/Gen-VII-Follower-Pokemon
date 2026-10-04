#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_ROOT="$SCRIPT_DIR/out"
if [[ "${DIAGNOSTIC:-0}" == "1" ]]; then
  SOURCE="$SCRIPT_DIR/Gen7FieldFollowerDiagnostic.3gx"
  OUTPUT="$OUTPUT_ROOT/diagnostic"
else
  SOURCE="$SCRIPT_DIR/Gen7FieldFollower.3gx"
  OUTPUT="$OUTPUT_ROOT/release"
fi

if [[ ! -f "$SOURCE" ]]; then
  printf 'Missing build artifact: %s\n' "$SOURCE" >&2
  exit 1
fi

for title_id in \
  0004000000164800 \
  0004000000175E00 \
  00040000001B5000 \
  00040000001B5100
do
  destination="$OUTPUT/luma/plugins/$title_id"
  mkdir -p "$destination"
  cp "$SOURCE" "$destination/Gen7FieldFollower.3gx"
  cp "$SCRIPT_DIR/assets/ride/"*.bin "$destination/"
  mkdir -p "$OUTPUT/archives"
  (
    cd "$OUTPUT"
    rm -f "archives/Gen7FieldFollower-$title_id.zip"
    zip -q -r "archives/Gen7FieldFollower-$title_id.zip" "luma/plugins/$title_id"
  )
done

printf '%s\n' "$OUTPUT"
