#!/usr/bin/env python3
"""
Pack files back into BURIKO ARC20 archive
Usage: python3 arc_pack.py <original.arc> <input_dir> <output.arc>
"""

import os
import sys
import struct
import re

def read_mapping(mapping_path):
    """Читает бинарный маппинг: safe_name -> original_name (bytes in CP932)"""
    mapping = {}
    if not os.path.exists(mapping_path):
        return mapping
    with open(mapping_path, 'rb') as f:
        data = f.read()
    pos = 0
    while pos < len(data):
        safe_len = struct.unpack('<I', data[pos:pos+4])[0]
        pos += 4
        safe_name = data[pos:pos+safe_len].decode('ascii')
        pos += safe_len
        orig_len = struct.unpack('<I', data[pos:pos+4])[0]
        pos += 4
        orig_name = data[pos:pos+orig_len]  # bytes in CP932
        pos += orig_len
        mapping[safe_name] = orig_name
    return mapping

def get_file_by_name(input_dir, orig_name_cp):
    """
    Ищет файл в папке по оригинальному имени (в CP932).
    Пробует разные варианты: с расширением и без.
    """
    # Преобразуем CP932 байты в строку для поиска в файловой системе
    # Имена на диске в UTF-8, поэтому конвертируем CP932 -> UTF-8
    try:
        utf8_name = orig_name_cp.decode('cp932')
    except:
        # Если не декодируется, пробуем как есть
        utf8_name = orig_name_cp.decode('utf-8', errors='replace')

    # Варианты имён для поиска
    candidates = [
        utf8_name,  # как есть
        utf8_name + '.dsc',
        utf8_name + '.cbg',
        utf8_name + '.bse',
        utf8_name + '.dat',
        utf8_name + '.png',  # для CBG
    ]

    for candidate in candidates:
        full_path = os.path.join(input_dir, candidate)
        if os.path.exists(full_path):
            with open(full_path, 'rb') as f:
                return f.read(), candidate

    # Пробуем найти файл с любым расширением
    for root, dirs, files in os.walk(input_dir):
        for f in files:
            # Проверяем, начинается ли имя файла с utf8_name
            if f.startswith(utf8_name) and f != utf8_name:
                full_path = os.path.join(root, f)
                with open(full_path, 'rb') as inf:
                    return inf.read(), f

    return None, None

def pack_arc(original_arc_path, input_dir, output_path):
    """Создаёт новый ARC-архив, используя файлы с оригинальными именами"""

    with open(original_arc_path, 'rb') as f:
        data = f.read()

    if data.startswith(b'PackFile    '):
        version = 1
    elif data.startswith(b'BURIKO ARC20'):
        version = 2
    else:
        raise ValueError("Not a valid ARC file")

    pos = 12
    count = struct.unpack('<I', data[pos:pos+4])[0]
    pos += 4

    # Читаем метаданные оригинального архива
    files_meta = []
    for i in range(count):
        name = data[pos:pos+16].split(b'\x00')[0]  # имя в CP932
        pos += 16
        if version == 1:
            offset = struct.unpack('<I', data[pos:pos+4])[0]
            size = struct.unpack('<I', data[pos+4:pos+8])[0]
            pos += 8
            pos += 8
        else:
            pos += 20*4
            offset = struct.unpack('<I', data[pos:pos+4])[0]
            size = struct.unpack('<I', data[pos+4:pos+8])[0]
            pos += 8
            pos += 6*4
        files_meta.append((name, offset, size))

    # Читаем маппинг из _mapping.dat
    mapping_path = os.path.join(input_dir, '_mapping.dat')
    mapping = read_mapping(mapping_path)
    print(f"Loaded {len(mapping)} entries from mapping")

    print(f"Packing {len(files_meta)} files from {input_dir}...")

    with open(output_path, 'wb') as f:
        # Заголовок
        if version == 1:
            f.write(b'PackFile    ')
        else:
            f.write(b'BURIKO ARC20')
        f.write(struct.pack('<I', count))

        all_file_data = []
        all_names = []
        used_files = 0
        missing_files = []

        for i, (orig_name, orig_offset, orig_size) in enumerate(files_meta):
            # Пробуем найти файл по оригинальному имени
            file_data, actual_name = get_file_by_name(input_dir, orig_name)

            if file_data is not None:
                used_files += 1
                # Используем оригинальное имя из архива
                name_to_use = orig_name
                print(f"  [{i:04d}] found: {actual_name}")
            else:
                missing_files.append(i)
                print(f"  [{i:04d}] WARNING: file not found")
                file_data = b''
                name_to_use = orig_name

            all_file_data.append(file_data)
            all_names.append(name_to_use)

        if missing_files:
            print(f"WARNING: {len(missing_files)} files not found")

        # Пишем метаданные
        meta_start = f.tell()

        for i, name in enumerate(all_names):
            name_bytes = name.ljust(16, b'\x00')[:16]
            f.write(name_bytes)

            if version == 1:
                f.write(struct.pack('<I', 0))
                f.write(struct.pack('<I', len(all_file_data[i])))
                f.write(b'\x00' * 8)
            else:
                f.write(b'\x00' * (20*4))
                f.write(struct.pack('<I', 0))
                f.write(struct.pack('<I', len(all_file_data[i])))
                f.write(b'\x00' * (6*4))

        # Пишем данные и обновляем offset'ы
        for i, file_data in enumerate(all_file_data):
            if len(file_data) == 0:
                continue

            current_pos = f.tell()

            if version == 1:
                offset_pos = meta_start + i * (16 + 4 + 4 + 8) + 16
            else:
                offset_pos = meta_start + i * (16 + 20*4 + 4 + 4 + 6*4) + 16 + 20*4

            f.seek(offset_pos)
            f.write(struct.pack('<I', current_pos))

            f.seek(current_pos)
            f.write(file_data)

        print(f"Done. Packed {used_files}/{len(files_meta)} files")
        print(f"Written {f.tell()} bytes to {output_path}")

def main():
    if len(sys.argv) != 4:
        print("Usage: python3 arc_pack.py <original.arc> <input_dir> <output.arc>")
        sys.exit(1)

    original_arc = sys.argv[1]
    input_dir = sys.argv[2]
    output_arc = sys.argv[3]

    if not os.path.exists(original_arc):
        print(f"Error: {original_arc} not found")
        sys.exit(1)

    if not os.path.exists(input_dir):
        print(f"Error: {input_dir} not found")
        sys.exit(1)

    try:
        pack_arc(original_arc, input_dir, output_arc)
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()
