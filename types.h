#include "ansi_codes.h"
#include "config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum Controls {
  // File
  EXPORT = 'e',
  CANCEL = 'c',

  // Cursor
  HIDE_CURSOR = 'i',
  H_LEFT = 'h',
  J_DOWN = 'j',
  K_UP = 'k',
  L_RIGHT = 'l',
  A_LEFT = 'a',
  S_DOWN = 's',
  W_UP = 'w',
  D_RIGHT = 'd',
  LEFT_EDGE = 'm',
  DOWN_EDGE = ',',
  UP_EDGE = '.',
  RIGHT_EDGE = '/',

  // Tools
  USE_TOOL = 'u',
  NEXT_TOOL = ';',
  SELECT_PENCIL = 'p',
  SELECT_BUCKET = 'b',

  // Colors
  NEXT_COLOR = 'n',
  PREV_COLOR = 'N',
  COLOR_1 = '1',
  COLOR_2 = '2',
  COLOR_3 = '3',
  COLOR_4 = '4',
  COLOR_5 = '5',
  COLOR_6 = '6',
  COLOR_7 = '7',
  COLOR_8 = '8',
  COLOR_9 = '9',
  COLOR_0 = '0',
};

enum Tool {
  PENCIL = 0,
  BUCKET,
};

enum Colors {
  E_WHITE = 0,
  E_BLACK,
  E_RED,
  E_ORANGE,
  E_YELLOW,
  E_GREEN,
  E_CYAN,
  E_BLUE,
  E_MAGENTA,
  E_BROWN,
};

enum Style {
  E_NONE = 0,
  E_BOLD,
  E_UNDERLINE,
  E_STRIKE,
};

enum ExportState { NO_EXPORT_YET = 0, EXPORT_SUCCEEDED, EXPORT_FAILED };

struct Screen {
  enum Colors cells[ROW_COUNT][COL_COUNT];
  uint8_t cursor_xy[2];
  bool is_cursor_hidden;
  enum Tool curr_tool;
  enum Colors curr_color;

  enum ExportState export_state;
  char *export_log;
};

enum SpecialKeys { BACKSPACE = 127, ENTER = 10 };

enum FileExtension { PNG = 0, JPG, ICO, PPM };

struct ExportFileInfo {
  char extension[5];
  char *name;
  size_t image_scale;
};

enum ColorMode{ BG_MODE = 0, FG_MODE };
