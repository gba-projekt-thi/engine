# Tilemap Workflow

- Create the Tileset in Aseprite [Prerequisites](#tilemap-prerequisites)
- Create a Tilemap in [Tiled](#tiled)
- Convert XML -> CPP header in [Python](#python)
- Import [Everything](#import-files-into-the-project)
- Create a Minimum [Main](#simple-main-function)

---

# Tilemap Prerequisites

**Prerequisite**: 
- The tileset as BMP file
- An 8x8 tile that is completely transparent
    - Butano calculates tiles always in 8x8.

Here: `graphics_files/simple_tileset.bmp` with tile 36 as transparent.

[Butano BMP files + JSON](https://gvaliente.github.io/butano/import.html)

---

# Tiled

[Website](https://www.mapeditor.org/)

**Workflow**
- New Map ...
    - Defaults: `Orthogonal`, `CSV`, `Right Down`
    - Map Size: Enter width / height of map
    - Tile Size: 8x8 (even with 16x16 tilesets)
- Bottom right under Tilesets hit `New Tileset ...`
    - Name: `your_name`
    - Source: `.../simple_tileset.bmp`
    - -> Save As
- Draw your Tilemap
- Save As ...

Here: `graphics_files/tilemap_8_8/tilemap.tmx`

# Python

### Variables
```python
EMPTY_BUTANO_INDEX  = 36
```
Tile of your empty 8x8 tile with the ID that Tiled shows in Properties.

### Run it
```sh
python .\convertcpp.py .\tilemap.tmx
```

### Result
Creates a `<name>_data.h` with the following data:

```cpp
constexpr int map_width  = 64;
constexpr int map_height = 64;
alignas(4) constexpr bn::regular_bg_map_cell map_cells[4096] = { /* ... */ };
constexpr int map_rect_count = 11;
constexpr MapRect map_rects[11] = { /* ... */ };
```

---

# Import Files into the Project

## `/graphics`

Create the tileset **twice**, once for the tiles and once for the palette.

`simple_tileset.bmp` with `{ "type": "regular_bg_tiles", "bpp_mode": "bpp_4" }`

`palette.bmp` with `{ "type": "bg_palette", "bpp_mode": "bpp_4" }`

## `/include`

Copy the Python header file into here.

---

# Simple Main Function

```cpp
// Create the map item from the generated tile data and dimensions
bn::regular_bg_map_item map_item(map_cells[0], bn::size(map_width, map_height));

// Combine tiles, palette, and map into one background item
bn::regular_bg_item bg_item(
    bn::regular_bg_tiles_items::simple_tileset,  // tile graphics from simple_tileset.bmp
    bn::bg_palette_items::palette,               // color palette from palette.bmp
    map_item);                                   // map layout from Python-generated header

// Create the background and place it at origin
bn::regular_bg_ptr bg = bg_item.create_bg(0, 0);
```
