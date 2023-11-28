#pragma once

#include <FS.h>

extern void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
extern void createDir(fs::FS &fs, const char *path);
extern void removeDir(fs::FS &fs, const char *path);
extern void readFile(fs::FS &fs, const char *path);
extern void writeFile(fs::FS &fs, const char *path, const char *message);
extern void appendFile(fs::FS &fs, const char *path, const char *message);
extern void renameFile(fs::FS &fs, const char *path1, const char *path2);
extern void deleteFile(fs::FS &fs, const char *path);
extern void testFileIO(fs::FS &fs, const char *path);
