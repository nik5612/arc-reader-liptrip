#!/bin/bash

# Путь к папке с архивами (где лежат .arc файлы)
SOURCE_DIR="/run/media/bystrovno/EFBD-7CE1/LipTrip"

# Базовая папка, где будут создаваться подпапки с извлечёнными данными
TARGET_BASE="/run/media/bystrovno/EFBD-7CE1/LipTrip/arc-reader-master"

# Проверяем, что ethornell существует и исполняем
if [ ! -x ./ethornell ]; then
    echo "Ошибка: ./ethornell не найден или не исполняемый"
    exit 1
fi

# Ищем все .arc файлы (только в корне SOURCE_DIR, не рекурсивно)
for arcfile in "$SOURCE_DIR"/*.arc; do
    if [ -f "$arcfile" ]; then
        # Имя архива без расширения
        basename=$(basename "$arcfile" .arc)
        target_dir="$TARGET_BASE/$basename"

        # Создаём папку для этого архива
        mkdir -p "$target_dir"
        echo "Извлечение $arcfile -> $target_dir"

        # Запускаем ethornell с двумя аргументами
        ./ethornell "$arcfile" "$target_dir"

        # Проверяем результат
        if [ $? -eq 0 ]; then
            echo "Успешно обработан $arcfile"
        else
            echo "Ошибка при обработке $arcfile"
        fi
    fi
done

echo "Готово."
