#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "arc.h"
#include "dsc.h"
#include "cbg.h"
#include "bse.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

/* ---- Создать все папки в пути (рекурсивно) ---- */
static int create_directories(const char *path)
{
	if (!path || !*path) return 0;
	char tmp[1024];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", path);
	len = strlen(tmp);
	if (tmp[len - 1] == '/') tmp[len - 1] = 0;
	for (p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = 0;
			MKDIR(tmp);
			*p = '/';
		}
	}
	MKDIR(tmp);
	return 0;
}

/* ---- Запись в mapping-файл ---- */
static void write_mapping(const char *dir_path, const char *safe_name, const char *orig_name)
{
	char map_path[1024];
	snprintf(map_path, sizeof(map_path), "%s/_mapping.txt", dir_path);
	FILE *f = fopen(map_path, "ab");  // append, binary mode
	if (f) {
		fprintf(f, "%s|%s\n", safe_name, orig_name);
		fclose(f);
	}
}

/* ---- Основная функция ---- */

int main(int argc, char **argv)
{
	if (argc != 2 && argc != 3) {
		puts("Usage: ethornell <file.arc> [output_dir]");
		return 1;
	}

	const char *output_base = (argc == 3) ? argv[2] : ".";

	struct Arc *arc = arc_open(argv[1]);
	if (!arc) {
		fprintf(stderr, "Error: cannot open archive '%s'\n", argv[1]);
		return 1;
	}

	uint32_t count = arc_files_count(arc);
	printf("number of files: %u\n", count);

	/* Создаём выходную папку, если указана */
	if (argc == 3) {
		create_directories(output_base);
	}

	int extracted = 0, errors = 0;

	for (uint32_t i = 0; i < count; i++) {
		uint8_t *raw_data = arc_get_file_data(arc, i);
		if (!raw_data) {
			fprintf(stderr, "Warning: failed to read file #%u\n", i);
			errors++;
			continue;
		}
		uint32_t filesize = arc_get_file_size(arc, i);
		uint8_t *bse_data = raw_data;
		int good = 1;
		const char *ext = ".dat";

		/* Получаем оригинальное имя (в CP932) */
		char *orig_name = arc_get_file_name(arc, i);

		/* Определяем тип и расширение */
		if (bse_is_valid(raw_data, filesize)) {
			ext = ".bse";
			if (bse_decrypt(raw_data)) {
				bse_data = raw_data + 16;
			}
		} else if (dsc_is_valid(raw_data, filesize)) {
			ext = ".dsc";
		} else if (cbg_is_valid(raw_data, filesize)) {
			ext = ".cbg";
		}

		/* Формируем безопасное имя файла: file_XXXX.ext */
		char safe_name[32];
		snprintf(safe_name, sizeof(safe_name), "file_%04u%s", i, ext);

		char full_path[1024];
		if (argc == 3)
			snprintf(full_path, sizeof(full_path), "%s/%s", output_base, safe_name);
		else
			snprintf(full_path, sizeof(full_path), "%s", safe_name);

		printf("%s...", safe_name); fflush(stdout);

		/* Если это DSC, расшифровываем и сохраняем */
		if (dsc_is_valid(bse_data, filesize)) {
			uint32_t dec_size;
			uint8_t *dec_data = dsc_decrypt(bse_data, filesize, &dec_size);
			if (dec_data) {
				good = dsc_save(dec_data, dec_size, full_path);
				free(dec_data);
			} else {
				fprintf(stderr, "dsc_decrypt failed for #%u\n", i);
				good = 0;
			}
		}
		/* Если CBG, расшифровываем в PNG (или сохраняем raw) */
		else if (cbg_is_valid(bse_data, filesize)) {
			uint16_t w, h;
			uint8_t *pixels = cbg_decrypt(bse_data, &w, &h);
			if (pixels) {
				good = cbg_save(pixels, w, h, full_path);
				free(pixels);
			} else {
				fprintf(stderr, "cbg_decrypt failed for #%u\n", i);
				good = 0;
			}
		}
		/* Остальное (BSE или неизвестное) сохраняем как есть */
		else {
			FILE *f = fopen(full_path, "wb");
			if (f) {
				fwrite(bse_data, filesize, 1, f);
				fclose(f);
			} else {
				fprintf(stderr, "fopen error for '%s': %s\n", full_path, strerror(errno));
				good = 0;
			}
		}

		/* Если файл успешно сохранён, записываем маппинг */
		if (good) {
			write_mapping(output_base, safe_name, orig_name);
			puts("ok");
			extracted++;
		} else {
			puts("ERROR");
			errors++;
		}

		free(raw_data);
	}

	arc_close(arc);
	printf("Done: %d extracted, %d errors\n", extracted, errors);
	return (errors > 0) ? 1 : 0;
}
