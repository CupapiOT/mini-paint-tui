#include "ansi_codes.h"
#include "colors.h"
#include "config.h"
#include "types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define clear_screen() printf(CLEAR_SCREEN)

typedef enum Controls Controls;
typedef enum Tool Tool;
typedef enum Colors Colors;
typedef enum Style Style;
typedef enum ColorMode ColorMode;
typedef struct Screen Screen;
typedef struct ExportFileInfo ExportFileInfo;

const uint8_t *color_palette[COLOR_COUNT] = {
    [E_WHITE] = WHT,   [E_BLACK] = BLK, [E_RED] = RED,  [E_ORANGE] = ORG,
    [E_YELLOW] = YEL,  [E_GREEN] = GRN, [E_CYAN] = CYN, [E_BLUE] = BLU,
    [E_MAGENTA] = MAG, [E_BROWN] = BRW};
const char *color_names[COLOR_COUNT] = {
    [E_WHITE] = "White",   [E_BLACK] = "Black",   [E_RED] = "Red",
    [E_ORANGE] = "Orange", [E_YELLOW] = "Yellow", [E_GREEN] = "Green",
    [E_CYAN] = "Cyan",     [E_BLUE] = "Blue",     [E_MAGENTA] = "Magenta",
    [E_BROWN] = "Brown"};
const char file_extensions[4][5] = {
    [PNG] = ".png\0",
    [JPG] = ".jpg\0",
    [ICO] = ".ico\0",
    [PPM] = ".ppm\0",
};

char *rgb_to_ansi(const ColorMode color_mode, const uint8_t *rgb) {
  size_t ansi_color_len =
      snprintf(NULL, 0, "%s%hu;%hu;%hum", ANSI_BACKGROUND_PREFIX, rgb[0],
               rgb[1], rgb[2]) +
      1;
  char *ansi_color = malloc(ansi_color_len);
  if (ansi_color == NULL) {
    fprintf(stderr, "Failed to malloc ansi_color in process_export_state "
                    "function. Aborting.");
  }
  snprintf(ansi_color, ansi_color_len, "%s%hu;%hu;%hum",
           color_mode == BG_MODE ? ANSI_BACKGROUND_PREFIX
                                 : ANSI_FOREGROUND_PREFIX,
           rgb[0], rgb[1], rgb[2]);
  return ansi_color;
}

void print_ansi(const char *background_color, const char *foreground_color,
                const char *content, const Style style) {
  const char *styles[4] = {"", BOLD, UNDERLINE, STRIKE};
  printf("%s%s%s%s%s", background_color, foreground_color, styles[style],
         content, RESET);
}

void print_screen(Screen *scr) {
  // Print title
  const uint8_t width = COL_COUNT * 2; // Number of chars
  const uint8_t title_len = strlen(TITLE) - 1;
  const int pad = (title_len >= width) ? 0 : (width - title_len - 2) / 2;
  printf("%*.*s%s\n", pad, pad, " ", TITLE);

  // Print canvas
  for (uint8_t row = 0; row < ROW_COUNT; row++) {
    for (uint8_t col = 0; col < COL_COUNT; col++) {
      const uint8_t cell = scr->cells[row][col];
      if (cell > (COLOR_COUNT - 1)) {
        printf("%02d", cell);
        continue;
      }

      if (scr->curr_tool == BUCKET) {
        print_ansi(rgb_to_ansi(BG_MODE, color_palette[cell]),
                   rgb_to_ansi(FG_MODE, color_palette[scr->curr_color]),
                   CURSOR_PIXEL, E_NONE);
        continue;
      }

      bool is_cursor_here =
          row == scr->cursor_xy[1] && col == scr->cursor_xy[0];
      if (is_cursor_here && !scr->is_cursor_hidden) {
        print_ansi(rgb_to_ansi(BG_MODE, color_palette[cell]),
                   rgb_to_ansi(FG_MODE, color_palette[scr->curr_color]),
                   CURSOR_PIXEL, E_NONE);
      } else {
        print_ansi(rgb_to_ansi(BG_MODE, color_palette[cell]),
                   rgb_to_ansi(FG_MODE, color_palette[cell]), FULL_PIXEL,
                   E_NONE);
      }
    }
    printf("\n");
  }

  // Status line
  //   Cursor
  printf("(%02hu, %02hu)", scr->cursor_xy[0] + 1, scr->cursor_xy[1] + 1);
  printf(" | ");
  //   Selected tool
  printf("Tool: %s", scr->curr_tool == 0 ? "Pencil" : "Bucket");
  printf(" | ");
  //   Selected color
  printf("Color: %-7s", color_names[scr->curr_color]);
  printf(" (");
  for (uint8_t i = 0; i < COLOR_COUNT; i++) {
    char color_letter[2];
    color_letter[0] = color_names[i][0];
    if (i != scr->curr_color) {
      print_ansi(RESET, rgb_to_ansi(FG_MODE, color_palette[i]), color_letter,
                 E_NONE);
    } else {
      // Any argument that isn't use may also be used to define a second style.
      print_ansi(BOLD, rgb_to_ansi(FG_MODE, color_palette[i]), color_letter,
                 E_UNDERLINE);
    }
  }
  printf(")\n");

  // Empty Line for controls info.
  printf("\n");
  /*
  `e` export     `c` cancel       `u` use tool     `p` pencil    `b` bucket
  `n` next color `N` prev color   `1-9` pick color `;` next tool `i` hide cursor
  `hjkl` l/d/u/r `m,./` go to l/d/u/r edge
  */
  printf("`e` export     `c` cancel       `u` use tool     `p` pencil    `b` "
         "bucket\n");
  printf("`n` next color `N` prev color   `1-9` pick color `;` next tool `i` "
         "hide cursor\n");
  printf("`hjkl` l/d/u/r `m,./` go to l/d/u/r edge\n\n");

  if (scr->export_log != NULL)
    printf("Last Export Log: %s\n\n", scr->export_log);
}

bool confirm_cancel_state() {
  printf("Do you want to exit (program cannot save)? y/n > ");
  while (true) {
    char keypress = getchar();
    if (keypress == 'n')
      return false;
    if (keypress == 'y')
      return true;
  }
}

bool export_file(ExportFileInfo *file_info, Screen *scr) {
  if (file_info == NULL || scr == NULL) {
    return false;
  }
  FILE *file = fopen(file_info->name, "wb");
  if (!file) {
    printf("Failed to open file: %s\n", file_info->name);
    return false;
  }

  const size_t w = COL_COUNT * file_info->image_scale;
  const size_t h = ROW_COUNT * file_info->image_scale;
  fprintf(file, "P6\n%zu %zu\n255\n", w, h);
  for (size_t row = 0; row < h; row++) {
    for (size_t col = 0; col < w; col++) {
      Colors cell_color = scr->cells[row % ROW_COUNT][col % COL_COUNT];
      for (uint16_t i = 0; i < file_info->image_scale; i++) {
        fputc(color_palette[cell_color][0], file);
        fputc(color_palette[cell_color][1], file);
        fputc(color_palette[cell_color][2], file);
      }
    }
  }

  size_t command_size = snprintf(NULL, 0, "ffmpeg -i %s %s%s", file_info->name,
                                 file_info->name, file_info->extension) +
                        1; // '\0'

  char *command_str = malloc(command_size);
  if (command_str == NULL) {
    fprintf(stderr,
            "Failed to malloc command_str in export_file function. Aborting.");
    return false;
  }
  snprintf(command_str, command_size, "ffmpeg -i %s %s%s", file_info->name,
           file_info->name, file_info->extension);
  system(command_str);
  fclose(file);
  command_size = snprintf(NULL, 0, "rm %s", file_info->name) + 1;
  command_str =
      realloc(command_str, command_size);
  snprintf(command_str, command_size, "rm %s", file_info->name);
  system(command_str);
  free(command_str);
  return true;
}

void process_export_state(Screen *scr) {
  typedef enum {
    SELECTING_FORMAT,
    NAMING_FILE,
    SPECIFY_SCALE,
  } ExportState;
  ExportState state = SELECTING_FORMAT;
  ExportFileInfo *file_info = malloc(sizeof(ExportFileInfo));
  if (file_info == NULL) {
    fprintf(stderr, "Failed to malloc file_info in process_export_state "
                    "function. Aborting.");
    return;
  }
  char *file_name = malloc(sizeof(char) * 16);
  if (file_name == NULL) {
    fprintf(stderr, "Failed to malloc file_name in process_export_state "
                    "function. Aborting.");
    return;
  }
  for (uint8_t i = 0; i < 16; i++)
    file_name[i] = '\0';
  size_t file_name_index = 0;
  file_info->name = file_name;
  file_info->image_scale = 1;

  const uint8_t max_scale_digits = 5;
  char image_scale_str[] = {'1', 0, 0, 0, 0};
  uint8_t image_scale_str_index = 1;
  char keypress;

  while (true) {
    clear_screen();
    print_screen(scr);

    // Logs
    printf("Exporting file (Press `c` to cancel):\n");
    printf("Select format:\n");
    printf("1. PNG\n");
    printf("2. JPG\n");
    printf("3. ICO\n");
    printf("4. PPM\n");
    printf("1/2/3/4 > ");
    if (state >= NAMING_FILE) {
      printf("%s\n", file_info->extension);
      printf("File name: %s", file_name);
    }
    if (state == SPECIFY_SCALE) {
      printf("\nBy what factor should your image be scaled by? (Max: 9999)\n");
      printf("Width : 32 * %zu = %zu\n", file_info->image_scale,
             COL_COUNT * file_info->image_scale);
      printf("Width : 32 * %zu = %zu\n", file_info->image_scale,
             ROW_COUNT * file_info->image_scale);
      printf("> %s", image_scale_str);
    }

    keypress = getchar();
    if (keypress == 'c' && state != NAMING_FILE)
      goto process_export_state_end;

    if (state == SELECTING_FORMAT) {
      keypress -= '0';
      if (keypress >= 1 && keypress <= 4) {
        state = NAMING_FILE;
        file_name_index = 0;
        snprintf(file_info->extension, 5, "%s", file_extensions[keypress - 1]);
      }
    } else if (state == NAMING_FILE) {
      if (keypress == BACKSPACE) {
        if (file_name_index > 0)
          file_name[--file_name_index] = '\0';
        continue;
      } else if (keypress == ENTER) {
        if (file_name_index > 0)
          state = SPECIFY_SCALE;
        continue;
      } else if (keypress == '/') {
        continue;
      }

      if (file_name_index + 1 >= strlen(file_name)) {
        file_name =
            realloc(file_name, (file_name_index + 1) * sizeof(char) * 2);
      }
      file_name[file_name_index++] = keypress;
    } else {
      if (keypress == BACKSPACE) {
        if (image_scale_str_index > 0)
          image_scale_str[--image_scale_str_index] = '\0';
      } else if (keypress == ENTER) {
        file_info->image_scale = atoi(image_scale_str);
        break;
      } else if (keypress < '0' || keypress > '9') { // Not a digit.
        continue;
      } else {
        if (image_scale_str_index + 1 >= max_scale_digits) {
          continue;
        }
        image_scale_str[image_scale_str_index++] = keypress;
      }

      if (atoi(image_scale_str) == 0)
        file_info->image_scale = 1;
      else
        file_info->image_scale = atoi(image_scale_str);
    }
  }

  size_t file_name_size =
      snprintf(NULL, 0, "%s%s", file_info->name, file_info->extension) +
      1; // '\0'
  char *file_name_full = malloc(file_name_size);
  if (file_name_full == NULL) {
    fprintf(stderr, "Failed to malloc file_name_full in process_export_state "
                    "function. Aborting.");
    goto process_export_state_end;
  }
  snprintf(file_name_full, file_name_size, "%s%s", file_info->name,
           file_info->extension);

  bool success = export_file(file_info, scr);
  if (success) {
    char base_success_log[] = "File %s successfully exported.";
    size_t success_log_len = strlen(base_success_log);
    char *success_log = malloc(success_log_len + file_name_size);
    if (success_log == NULL) {
      fprintf(stderr, "Failed to malloc success_log in process_export_state "
                      "function. Aborting.");
      goto process_export_state_after_info_malloc_end;
    }
    snprintf(success_log, success_log_len + file_name_size, base_success_log,
             file_name_full);
    scr->export_log = success_log;
  } else {
    char base_fail_log[] = "File %s was not exported successfully.";
    size_t fail_log_len = strlen(base_fail_log);
    char *fail_log = malloc(fail_log_len + file_name_size);
    if (fail_log == NULL) {
      fprintf(stderr, "Failed to malloc fail_log in process_export_state "
                      "function. Aborting.");
      goto process_export_state_after_info_malloc_end;
    }
    snprintf(fail_log, fail_log_len + file_name_size, base_fail_log,
             file_name_full);
    scr->export_log = fail_log;
  }

process_export_state_after_info_malloc_end:
  free(file_name_full);
process_export_state_end:
  free(file_name);
  free(file_info);
}

int decrement_wrap(const int val, const int max_val) {
  return val == 0 ? max_val : val - 1;
}

int main() {
  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  Screen scr;
  for (uint16_t row = 0; row < ROW_COUNT; row++) {
    for (uint16_t col = 0; col < COL_COUNT; col++) {
      scr.cells[row][col] = E_WHITE;
    }
  }
  scr.curr_color = E_BLACK;

  while (true) {
    clear_screen();
    print_screen(&scr);
    char keypress = getchar();

    if (keypress == CANCEL) {
      if (confirm_cancel_state())
        break;
      continue;
    }

    if (keypress == EXPORT) {
      process_export_state(&scr);
    }

    // Manual color selection.
    if (keypress == '0') {
      scr.curr_color = E_BROWN;
      continue;
    } else if (keypress >= '1' && keypress <= '9') {
      scr.curr_color = keypress - '0' - 1;
      continue;
    }

    // Other trivial-case keys.
    switch (keypress) {
    // Cursor
    case HIDE_CURSOR:
      scr.is_cursor_hidden = scr.is_cursor_hidden ? false : true;
      break;

    // Tools
    case USE_TOOL:
      if (scr.curr_tool == BUCKET) {
        for (uint16_t row = 0; row < ROW_COUNT; row++) {
          for (uint16_t col = 0; col < COL_COUNT; col++) {
            scr.cells[row][col] = scr.curr_color;
          }
        }
      } else if (scr.curr_tool == PENCIL) {
        scr.cells[scr.cursor_xy[1]][scr.cursor_xy[0]] = scr.curr_color;
      }
      break;
    case SELECT_PENCIL:
      scr.curr_tool = PENCIL;
      break;
    case SELECT_BUCKET:
      scr.curr_tool = BUCKET;
      break;
    case NEXT_TOOL:
      scr.curr_tool = (scr.curr_tool + 1) % TOOL_COUNT;
      break;

    // Next and Previous Colors (manual color selection is above)
    case NEXT_COLOR:
      scr.curr_color = (scr.curr_color + 1) % COLOR_COUNT;
      break;
    case PREV_COLOR:
      scr.curr_color = decrement_wrap(scr.curr_color, 9);
      break;

    // Movement
    //   Normal
    case H_LEFT:
    case A_LEFT:
      scr.cursor_xy[0] = decrement_wrap(scr.cursor_xy[0], COL_COUNT - 1);
      break;
    case J_DOWN:
    case S_DOWN:
      scr.cursor_xy[1] = (scr.cursor_xy[1] + 1) % ROW_COUNT;
      break;
    case K_UP:
    case W_UP:
      scr.cursor_xy[1] = decrement_wrap(scr.cursor_xy[1], ROW_COUNT - 1);
      break;
    case L_RIGHT:
    case D_RIGHT:
      scr.cursor_xy[0] = (scr.cursor_xy[0] + 1) % COL_COUNT;
      break;
    //   To-edge
    case LEFT_EDGE:
      scr.cursor_xy[0] = 0;
      break;
    case DOWN_EDGE:
      scr.cursor_xy[1] = ROW_COUNT - 1;
      break;
    case UP_EDGE:
      scr.cursor_xy[1] = 0;
      break;
    case RIGHT_EDGE:
      scr.cursor_xy[0] = COL_COUNT - 1;
      break;

    // Ignore other keys
    default:
      break;
    }
  }
  clear_screen();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return EXIT_SUCCESS;
}
