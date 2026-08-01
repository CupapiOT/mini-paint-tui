# Mini Paint TUI

A small TUI that lets you create 32x32 px images (scalable by n factor) with 10
different colors and two tools.

![paint-tui-showcase](https://github.com/user-attachments/assets/928300c5-241a-4cac-b0d2-21eb5260f602)

## Scope

**Features of this TUI**:

1. Initiates with a UI that shows the:
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
4. Can export to new `.ppm`, `.png`, or `.jpg` file with a given name, with the
   option to scale the image by any integer from 1&ndash;9999.
6. Can cancel file (with y/n confirmation)
7. The cursor may be hidden, or visible, marked by textured pixels.
8. Each pixel is 1 character in height and 2 characters in width.

**Limitations**:

1. Cannot save nor load files in progress
2. Cannot create files with screen sizes other than 32x32 px
3. Cannot use arbitrary colors

---

## Stack

- `C` language (C99)
- `termios` built-in library.
- `ffmpeg` CLI tool (used for exporting files into non-PPM formats).

---

## Controls

1. **File**:

   - `e`: export (command line asks to specify name and extension)
   - `c`: cancel (command line prompts for confirmation)

2. **Cursor**:

   - `i`: hide cursor
   - `h`: left
   - `j`: down
   - `k`: up
   - `l`: right
   - `m`: go to left edge
   - `,`: go to down edge
   - `.`: go to up edge
   - `/`: go to right edge

3. **Tools**:

   - `p`: Select pencil
   - `b`: Select bucket
   - `;`: Next tool
   - `u`: Use tool

4. **Select Color**:

   - `n`: Next color (loops around)
   - `N`: Previous color (loops around)
   - `1`: White,
   - `2`: Black,
   - `3`: Red,
   - `4`: Orange,
   - `5`: Yellow,
   - `6`: Green,
   - `7`: Cyan,
   - `8`: Blue,
   - `9`: Magenta,
   - `0`: Brown,



---

## UI Breakdown

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
----------------------------------------------------------------
// x & y are cursor coordinates
(x, y) | Tool: Pencil | Color: white (WBROYGCBMB) // selected color will be underlined; letters are color-coded

`e` export     `c` cancel       `u` use tool     `p` pencil    `b` bucket
`n` next color `N` prev color   `1-9` pick color `;` next tool `i` hide cursor
`hjkl` l/d/u/r `m,./` go to l/d/u/r edge

Last Export Log: File example.png successfully exported. // Visible after exporting file

Cancel? y/n >             // command line (pops up on export & discard commands)
```

## Acknowledgements

Inspired by the video [Graphics API is irrelevant](https://youtu.be/xNX9H_ZkfNE) by
[Tsoding on YouTube](https://www.youtube.com/@Tsoding), which showcases how animations can be 
made from raw pixels which are then manipulated with algorithms.
