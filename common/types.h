#ifndef PLATYPUS_TYPES_H
#define PLATYPUS_TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Universal Consts
#define MAX_PAYLOADS 10
#define MAX_FILENAME_LEN 64
#define MAX_VARIABLES 16
#define LCD_WIDTH   240
#define LCD_HEIGHT  240

// Colors
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_BLUE  0x001F
#define COLOR_GRAY  0x8410
#define COLOR_GREEN 0x07E0

// Data Structures
typedef enum { LANG_US, LANG_ES } KeymapLang;

typedef struct {
    unsigned char modifier;
    unsigned char scancode;
} HID_Lookup;

typedef struct {
    char name[32];
    int value;
} Variable;

typedef struct {
    int inside_if;
    int condition_met;
    int executing;
    long loop_start_pos;
    long for_start_pos;
    char for_var_name[32];
    int for_end_value;
    int line_number;
    KeymapLang active_lang;
    Variable vars[MAX_VARIABLES];
    int var_count;
} ParseContext;

// Global Shared Objects
extern int global_cached_os;
extern char payload_list[MAX_PAYLOADS][MAX_FILENAME_LEN];
extern int payload_count;

// Function Prototypes
void load_payload_directory(const char *dir_path);
HID_Lookup get_hid_code(char c, KeymapLang lang);
int get_utf8_codepoint(const char *str, int *bytes_consumed);
int get_var(ParseContext *ctx, const char *name);
void set_var(ParseContext *ctx, const char *name, int value);

void parse_line(const char *line, void *req, ParseContext *ctx, void *script_file);

#endif
