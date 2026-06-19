#!/bin/bash

# Папка, где лежит этот скрипт (arc-reader-master)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Папка с оригинальными .arc файлами
BASE="/run/media/bystrovno/EFBD-7CE1/LipTrip"

# Папка для сохранения новых архивов
PACKED_DIR="$SCRIPT_DIR/packed"
mkdir -p "$PACKED_DIR"

echo "Script directory: $SCRIPT_DIR"
echo "Looking for .arc files in: $BASE"
echo "Output directory: $PACKED_DIR"

for arcfile in "$BASE"/*.arc; do
    # Пропускаем, если нет файлов
    [ -e "$arcfile" ] || continue

    basename=$(basename "$arcfile" .arc)
    input_dir="$SCRIPT_DIR/$basename"

    if [ -d "$input_dir" ]; then
        # Сохраняем новый архив в папку packed с тем же именем (без _new)
        output="$PACKED_DIR/$basename.arc"
        echo "Packing $basename -> $output"
        python3 "$SCRIPT_DIR/arc_pack.py" "$arcfile" "$input_dir" "$output"
    else
        echo "Warning: $input_dir not found, skipping $basename"
    fi
done

echo "Done. All packed archives are in: $PACKED_DIR"
