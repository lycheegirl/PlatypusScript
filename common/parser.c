#include "types.h"
#include "platypus.h"

#if defined(PICO_ON_DEVICE) && PICO_ON_DEVICE == 1
    #include "pico/stdlib.h"
    #include "ff.h" 
    #define PLATFORM_SLEEP(ms) sleep_ms(ms)
    #define PLATFORM_FTELL(f)  ((long)f_tell((FIL *)f))
    #define PLATFORM_FSEEK(f, pos) f_lseek((FIL *)f, (FSIZE_t)pos)
#else
    #include <unistd.h>
    #include <stdio.h>
    #define PLATFORM_SLEEP(ms) usleep(ms * 1000)
    #define PLATFORM_FTELL(f)  ftell((FILE *)f)
    #define PLATFORM_FSEEK(f, pos) fseek((FILE *)f, pos, SEEK_SET)
#endif

int get_var(ParseContext *ctx, const char *name) {
    for (int i = 0; i < ctx->var_count; i++) {
        if (strcmp(ctx->vars[i].name, name) == 0) return ctx->vars[i].value;
    }
    return 0;
}

void set_var(ParseContext *ctx, const char *name, int value) {
    for (int i = 0; i < ctx->var_count; i++) {
        if (strcmp(ctx->vars[i].name, name) == 0) { ctx->vars[i].value = value; return; }
    }
    if (ctx->var_count < MAX_VARIABLES) {
        snprintf(ctx->vars[ctx->var_count].name, sizeof(ctx->vars[ctx->var_count].name), "%.31s", name);
        ctx->vars[ctx->var_count].value = value; 
        ctx->var_count++;
    }
}

int evaluate_condition(void *req, const char *cond, ParseContext *ctx) {
    if (strncmp(cond, "JOY_UP", 6) == 0) return hw_get_pin_value(req, PIN_JOYSTICK_UP);
    if (strncmp(cond, "JOY_DOWN", 8) == 0) return hw_get_pin_value(req, PIN_JOYSTICK_DOWN);
    if (strncmp(cond, "JOY_PRESS", 9) == 0) return hw_get_pin_value(req, PIN_JOYSTICK_PRESS);

    char var_name[32], op[4]; int target_val;
    if (sscanf(cond, "%31s %3s %d", var_name, op, &target_val) == 3) {
        int current_val = get_var(ctx, var_name);
        if (strcmp(op, "==") == 0) return current_val == target_val;
        if (strcmp(op, "!=") == 0) return current_val != target_val;
        if (strcmp(op, "<") == 0)  return current_val < target_val;
        if (strcmp(op, ">") == 0)  return current_val > target_val;
    }
    return 0;
}

void parse_line(const char *line, void *req, ParseContext *ctx, void *script_file) {
    ctx->line_number++;
    while (*line == ' ' || *line == '\t') line++;

    if (strncmp(line, "LANG ", 5) == 0) {
        if (ctx->line_number == 1) {
            char choice[16];
            if (sscanf(line + 5, "%15s", choice) == 1) {
                if (strcmp(choice, "ES") == 0) ctx->active_lang = LANG_ES;
                else ctx->active_lang = LANG_US;
            }
        }
        return;
    }

    if (strncmp(line, "SET ", 4) == 0) {
        if (!ctx->executing) return;
        char var_name[32], val_str[32];
        if (sscanf(line + 4, "%31s %31s", var_name, val_str) == 2) {
            if (val_str[0] >= '0' && val_str[0] <= '9') set_var(ctx, var_name, atoi(val_str));
            else set_var(ctx, var_name, get_var(ctx, val_str));
        }
        return;
    }

    if (strncmp(line, "IF ", 3) == 0) { ctx->inside_if = 1; int res = evaluate_condition(req, line + 3, ctx); ctx->executing = res; ctx->condition_met = res; return; }
    if (strncmp(line, "ELSE IF ", 8) == 0) { if (ctx->inside_if) { if (ctx->condition_met) ctx->executing = 0; else { int res = evaluate_condition(req, line + 8, ctx); ctx->executing = res; ctx->condition_met = res; } } return; }
    if (strncmp(line, "ELSE", 4) == 0) { if (ctx->inside_if) ctx->executing = !ctx->condition_met; return; }
    if (strncmp(line, "ENDIF", 5) == 0) { ctx->inside_if = 0; ctx->condition_met = 0; ctx->executing = 1; return; }
    if (strncmp(line, "DO", 2) == 0) { if (ctx->executing) ctx->loop_start_pos = PLATFORM_FTELL(script_file); return; }
    if (strncmp(line, "WHILE ", 6) == 0) { if (ctx->executing && ctx->loop_start_pos != -1) { if (evaluate_condition(req, line + 6, ctx)) PLATFORM_FSEEK(script_file, ctx->loop_start_pos); else ctx->loop_start_pos = -1; } return; }

    if (strncmp(line, "FOR ", 4) == 0) {
        if (!ctx->executing) return;
        char var[32]; int start, end;
        if (sscanf(line + 4, "%31s %d TO %d", var, &start, &end) == 3) {
            if (ctx->for_start_pos == -1) { 
                set_var(ctx, var, start); 
                ctx->for_start_pos = PLATFORM_FTELL(script_file); 
                snprintf(ctx->for_var_name, sizeof(ctx->for_var_name), "%.31s", var); 
                ctx->for_end_value = end; 
            }
        }
        return;
    }
    if (strncmp(line, "ENDFOR", 6) == 0) {
        if (!ctx->executing) return;
        if (ctx->for_start_pos != -1) {
            int cur = get_var(ctx, ctx->for_var_name) + 1; set_var(ctx, ctx->for_var_name, cur);
            if (cur <= ctx->for_end_value) PLATFORM_FSEEK(script_file, ctx->for_start_pos); else ctx->for_start_pos = -1;
        }
        return;
    }

    if (!ctx->executing) return;
    if (strncmp(line, "SLEEP ", 6) == 0) {
        // ... sleep logic ...
        return;
    }
    
    if (strncmp(line, "REM", 3) == 0 || strlen(line) <= 1) return;
    
    if (strncmp(line, "STRING ", 7) == 0) {
        const char *text = line + 7; int i = 0;
        while (text[i] != '\0' && text[i] != '\n') {
            int consumed = 0, cp = get_utf8_codepoint(&text[i], &consumed);
            // Pass req instead of fd to give the platform header full context
            hw_send_unicode(req, cp, get_var(ctx, "_OS"), ctx->active_lang);
            i += consumed;
        }
    }
}
