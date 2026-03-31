import sys
import xml.etree.ElementTree as ET

if len(sys.argv) < 2:
    print("Usage: python convert_map.py yourmap.tmx")
    sys.exit(1)

tmx_file = sys.argv[1]
output_file = tmx_file.replace('.tmx', '_data.h')

tree = ET.parse(tmx_file)
root = tree.getroot()

width = int(root.attrib['width'])
height = int(root.attrib['height'])

layer = root.find('layer')
data = layer.find('data').text.strip()
tiled_ids = [int(x) for x in data.split(',')]

EMPTY_BUTANO_INDEX = 36

map_cells = []
collision = []
for tid in tiled_ids:
    if tid == 0:
        map_cells.append(EMPTY_BUTANO_INDEX)
        collision.append(0)
    else:
        map_cells.append(tid - 1)
        collision.append(1)

def to_screenblock_order(cells, width, height):
    result = []
    for sb_y in range(height // 32):
        for sb_x in range(width // 32):
            for row in range(32):
                for col in range(32):
                    src_row = sb_y * 32 + row
                    src_col = sb_x * 32 + col
                    result.append(cells[src_row * width + src_col])
    return result

map_cells_sb = to_screenblock_order(map_cells, width, height)

with open(output_file, 'w') as f:
    f.write("#pragma once\n\n")
    f.write("#include \"bn_regular_bg_map_cell.h\"\n\n")
    f.write(f"constexpr int map_width = {width};\n")
    f.write(f"constexpr int map_height = {height};\n\n")

    f.write(f"alignas(4) constexpr bn::regular_bg_map_cell map_cells[{width * height}] = {{\n")
    for i in range(0, len(map_cells_sb), width):
        f.write("    ")
        for j in range(width):
            f.write(f"{map_cells_sb[i + j]},")
        f.write("\n")
    f.write("};\n\n")

    f.write(f"constexpr int collision_map[{height}][{width}] = {{\n")
    for y in range(height):
        f.write("    {")
        for x in range(width):
            idx = y * width + x
            f.write(f"{collision[idx]},")
        f.write("},\n")
    f.write("};\n")

print(f"Done! {width}x{height} map written to {output_file}")