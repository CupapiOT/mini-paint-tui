# Mini Paint TUI

A very small TUI that lets you create small 32x32 images.

## UI

Text with the prefix `//` will not be shown in the app.

```txt
                       Mini Paint TUI                   // title

----------------------------------------------------------------
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|           CANVAS (64x32 chars; looks like 32x32 px)          |
|                 (Will look off on GitHub)                    |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
|                                                              |
----------------------------------------------------------------
(x, y) | Tool: Pencil | Color: white (WBROYGCBMB) // selected color will be underlined; letters are color-coded

`e` export     `c` cancel       `u` use tool     `p` pencil    `b` bucket
`n` next color `N` prev color   `1-9` pick color `;` next tool `i` hide cursor
`hjkl` l/d/u/r `m,./` go to l/d/u/r edge

Last Export Log: File example.png successfully exported. // Visible after exporting file

Cancel? y/n >             // command line (pops up on export & discard commands)
```

## Scope

Features of this TUI:

1. Can initiate, with a UI that shows the:
   - canvas (32x32 px),
   - currently selected tool,
   - currently selected color,
   - all the controls,
   - coordinates of cursor,
   - command line where cancel-confirmation and export-name and
     export-extension-selection happens.
2. Pencil and naive bucket tool (fills whole screen)
3. Can draw with these colors: White, Black, Red, Orange, Yellow, Green, Cyan,
   Blue, Magenta, Brown
4. Can export to new `.ppm`, `.png`, or `.jpg` file with a given name.
5. Can cancel file (with y/n confirmation)
6. The cursor may be hidden, or may be visible, marked by textured pixels.
7. Each pixel is 1-char in height and 2-chars in width.

Things it may be able to do in the future:

1. Save and load file in progress
2. Open files with screen sizes other than 32x32
3. Have arbitrary colors (probably 24-bit)
4. Move entire image in any direction in the canvas (adjusting the image's
   position in the canvas)

---

## Controls

File:

- e: export (command line asks to specify name and extension)
- c: cancel (command line prompts for confirmation)

Cursor:

- i: hide cursor
- h: left
- j: down
- k: up
- l: right
- m: go to left edge
- ,: go to down edge
- .: go to up edge
- /: go to right edge

Tools:

- p: Select pencil
- b: Select bucket
- ;: Next tool
- u: Use tool

Select Color:

- n: Next color (loops around)
- N: Previous color (loops around)
- 1: White,
- 2: Black,
- 3: Red,
- 4: Orange,
- 5: Yellow,
- 6: Green,
- 7: Cyan,
- 8: Blue,
- 9: Magenta,
- 0: Brown,

---

## Stack

- `C` language (C99)
- `termios` built-in library.
