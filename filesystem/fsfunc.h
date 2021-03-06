#pragma once

#include "common.h"
#include "global.h"

void build_all_tree();

struct File* build_tree(struct File* fa, int32_t clusID);

void list_directory(struct File* dir, bool ShowHid);

void tree_directory(struct File* dir, bool ShowHid, int maxLayer);

void tree_directory_with_spacenum(int SpaceNum, struct File* p, bool ShowHid, int maxLayer);

struct File* is_file_exist(struct File* dir, const char* filename);

bool create_empty_file(struct File* dir, char* Filename, uint8_t Attr, uint16_t WrtTime, uint16_t WrtDate);

bool create_empty_directory(struct File* dir, char* Filename, uint16_t WrtTime, uint16_t WrtDate);

// bool update_buff_list(struct ActiveFile* acfile, struct AFBLNode** ptr_to_now, uint8_t* buff, int* ptr_to_clusID);

bool write_file(struct ActiveFile* acfile);

void read_file(struct ActiveFile* acfile);

bool delete_file_from_fa(struct File* fa, struct File* file);

bool delete_file(struct File* dir, struct File* file);

bool is_dir_empty(struct File* dir);

bool delete_directory(struct File* fa, struct File* dir, bool rc);

struct File* change_directory(struct File* dir, const char* subdirname);

bool append_file(struct File* file1, struct File* file2);

void shell_help();

void shell_ls(struct File* nowDir);

void shell_tree(struct File* nowDir);

void shell_cd(struct File** ptr_to_nowDir, char* nowPath);

char to_upper(const char c);

void standard_dir_path(char* s, const char* dirPath);

void divede_path(const char* dirPath, char* fstDir, char* resPath);

void fa_dir_path(char* fapath, const char* dirPath);

bool try_cd(const char* dirPath, struct File** ptr_to_nowDir, char* nowPath);

void file_path_to_dir_path(const char* filePath, char* dirPath, char* filename);

void shell_touch(struct File* nowDir);

void shell_mkdir(struct File* nowDir);

void shell_cat(struct File* nowDir);

void shell_write(struct File* nowDir, bool from_sda, uint32_t filesize);

void shell_append(struct File* nowDir);

void shell_rm(struct File* nowDir);

void shell_rmdir(struct File* nowDir);

void shell_exec(struct File* nowDir);

void shell_write_program_to_sdb();

// int32_t read_file_with_offset_and_size(struct ActiveFile* acfile, uint32_t offset, uint32_t size, void* dst);