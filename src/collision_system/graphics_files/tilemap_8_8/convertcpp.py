import sys
import xml.etree.ElementTree as ET

# =============================================================================
# Configuration — adjust these to match your project
# =============================================================================

EMPTY_BUTANO_INDEX  = 36        # Butano tile index used for empty/transparent tiles
OUTPUT_FILENAME     = None      # set to a string like "tilemap_data.h" to override,
                                # None -> (e.g. mymap.tmx → mymap_data.h)

TILE_WIDTH          = 8         # pixel width of one tile (Butano uses 8x8)
TILE_HEIGHT         = 8         # pixel height of one tile
DEFAULT_LAYERS      = 0x8000    # collision layer mask (bit 15 = tilemap)

# =============================================================================
# CLI
# =============================================================================

if len(sys.argv) < 2:
    print("Usage: python convertcpp.py yourmap.tmx")
    sys.exit(1)

tmx_file = sys.argv[1]

if OUTPUT_FILENAME is not None:
    output_file = OUTPUT_FILENAME
else:
    output_file = tmx_file.replace('.tmx', '_data.h')

# =============================================================================
# Parse TMX
# =============================================================================

tree = ET.parse(tmx_file)
root = tree.getroot()

width  = int(root.attrib['width'])
height = int(root.attrib['height'])

layer = root.find('layer')
data  = layer.find('data').text.strip()
tiled_ids = [int(x) for x in data.split(',')]

# =============================================================================
# Build map_cells (visual) and collision grid
# =============================================================================

map_cells = []
collision  = []          # flat list, row-major: 0 = passable, 1 = solid

for tid in tiled_ids:
    if tid == 0:
        map_cells.append(EMPTY_BUTANO_INDEX)
        collision.append(0)
    else:
        map_cells.append(tid - 1)
        collision.append(1)

# helper: read collision grid by (row, col)
def solid(row, col):
    return collision[row * width + col] == 1

# =============================================================================
# Screenblock reorder (unchanged from original)
# =============================================================================

def to_screenblock_order(cells, w, h):
    result = []
    for sb_y in range(h // 32):
        for sb_x in range(w // 32):
            for row in range(32):
                for col in range(32):
                    src_row = sb_y * 32 + row
                    src_col = sb_x * 32 + col
                    result.append(cells[src_row * w + src_col])
    return result

map_cells_sb = to_screenblock_order(map_cells, width, height)

# =============================================================================
# Greedy scanline rectangle merge
#
# Walks every tile once.  For each unvisited solid tile it:
#   1. expands RIGHT as far as the row stays solid
#   2. expands DOWN  as long as the full width stays solid
#   3. marks every covered tile as visited
#   4. emits one MapRect in world-pixel coordinates
# =============================================================================

visited = [[False] * width for _ in range(height)]
rects   = []   # list of (x_min, y_min, x_max, y_max)

for row in range(height):
    for col in range(width):
        if not solid(row, col) or visited[row][col]:
            continue

        # --- expand right ---
        end_col = col
        while end_col + 1 < width and solid(row, end_col + 1) and not visited[row][end_col + 1]:
            end_col += 1

        # --- expand down ---
        end_row = row
        while end_row + 1 < height:
            full_width = True
            for c in range(col, end_col + 1):
                if not solid(end_row + 1, c) or visited[end_row + 1][c]:
                    full_width = False
                    break
            if not full_width:
                break
            end_row += 1

        # --- mark visited ---
        for r in range(row, end_row + 1):
            for c in range(col, end_col + 1):
                visited[r][c] = True

        # --- emit rect in world pixels ---
        x_min = col            * TILE_WIDTH
        y_min = row            * TILE_HEIGHT
        x_max = (end_col + 1)  * TILE_WIDTH
        y_max = (end_row + 1)  * TILE_HEIGHT
        rects.append((x_min, y_min, x_max, y_max))

print(f"Merged {sum(collision)} solid tiles into {len(rects)} rectangles")

# =============================================================================
# Write output header
# =============================================================================

with open(output_file, 'w') as f:
    f.write("#pragma once\n\n")
    f.write("#include \"bn_regular_bg_map_cell.h\"\n\n")

    # --- map dimensions ---
    f.write(f"constexpr int map_width  = {width};\n")
    f.write(f"constexpr int map_height = {height};\n\n")

    # --- visual map_cells (screenblock order) ---
    f.write(f"alignas(4) constexpr bn::regular_bg_map_cell map_cells[{width * height}] = {{\n")
    for i in range(0, len(map_cells_sb), width):
        f.write("    ")
        for j in range(width):
            f.write(f"{map_cells_sb[i + j]},")
        f.write("\n")
    f.write("};\n\n")

    # --- merged collision rects ---
    f.write(f"constexpr int map_rect_count = {len(rects)};\n\n")
    f.write(f"constexpr MapRect map_rects[{len(rects)}] = {{\n")
    for (x0, y0, x1, y1) in rects:
        f.write(f"    {{ {x0}, {y0}, {x1}, {y1}, 0x{DEFAULT_LAYERS:04X} }},\n")
    f.write("};\n")

print(f"Done! {width}x{height} map written to {output_file}")