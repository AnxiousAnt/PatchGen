import re

FONT_INFO = [
    (8, 6, 10), (10, 7, 13), (12, 9, 16),
    (16, 10, 20), (24, 15, 30), (36, 25, 45)
]

def get_font_metrics(fontsize):
    for size, width, height in FONT_INFO:
        if fontsize <= size:
            return width, height
    return FONT_INFO[-1][1], FONT_INFO[-1][2]  # Return largest if no match

def parse_pd_file(file_path):
    elements = []
    fontsize = 10  # Default font size
    char_width, char_height = get_font_metrics(fontsize)

    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith('#N canvas'):
                parts = line.split()
                if len(parts) >= 7:
                    fontsize = int(parts[6].replace(";", ''))
                    char_width, char_height = get_font_metrics(fontsize)
            elif line.startswith('#X obj') or line.startswith('#X msg'):
                parts = line.split()
                if len(parts) >= 5:
                    element_type = 'obj' if line.startswith('#X obj') else 'msg'
                    x, y = int(parts[2]), int(parts[3])
                    content = parts[4]
                    
                    if content in ['bng', 'tgl', 'nbx', 'vsl', 'hsl', 'hradio', 'vradio']:
                        # Special case for GUI objects
                        if content in ['bng', 'tgl']:
                            width = height = int(parts[5])
                        elif content == 'nbx':
                            width = int(parts[5]) * char_width
                            height = int(parts[6])
                        elif content == 'vsl':
                            width = int(parts[5])
                            height = int(parts[6])
                        elif content == 'hsl':
                            width = int(parts[5])
                            height = int(parts[6])
                        elif content in ['hradio', 'vradio']:
                            size = int(parts[5])
                            count = int(parts[7])
                            if content == 'hradio':
                                width = size * count
                                height = size
                            else:  # vradio
                                width = size
                                height = size * count
                    else:
                        # For other objects and messages
                        content = ' '.join(parts[4:])
                        width = max(len(content) * char_width, char_width)  # Ensure minimum width
                        height = char_height
                    
                    elements.append({
                        'type': element_type,
                        'x': x,
                        'y': y,
                        'width': width,
                        'height': height,
                        'content': content,
                        'line': line.strip()
                    })
    return elements, fontsize

def check_overlap(elem1, elem2):
    return (elem1['x'] < elem2['x'] + elem2['width'] and
            elem1['x'] + elem1['width'] > elem2['x'] and
            elem1['y'] < elem2['y'] + elem2['height'] and
            elem1['y'] + elem1['height'] > elem2['y'])

def find_overlaps(elements):
    overlaps = []
    for i, elem1 in enumerate(elements):
        for elem2 in elements[i+1:]:
            if check_overlap(elem1, elem2):
                overlaps.append((elem1, elem2))
    return overlaps

def get_bounding_box(elem):
    return (elem['x'], elem['y'], elem['x'] + elem['width'], elem['y'] + elem['height'])

def main(file_path):
    elements, fontsize = parse_pd_file(file_path)
    overlaps = find_overlaps(elements)
    
    print(f"Patch font size: {fontsize}")
    char_width, char_height = get_font_metrics(fontsize)
    print(f"Character metrics: width = {char_width}, height = {char_height}")
    print()
    
    print("All elements:")
    for elem in elements:
        bb = get_bounding_box(elem)
        print(f"{elem['type']} at ({elem['x']}, {elem['y']}) - {elem['content']}")
        print(f"  Bounding box: ({bb[0]}, {bb[1]}) to ({bb[2]}, {bb[3]})")
        print(f"  Width: {elem['width']}, Height: {elem['height']}")
        print()
    
    if overlaps:
        print(f"\nFound {len(overlaps)} overlapping pairs:")
        for elem1, elem2 in overlaps:
            bb1 = get_bounding_box(elem1)
            bb2 = get_bounding_box(elem2)
            print(f"Overlap detected between:")
            print(f"  1: {elem1['type']} at ({elem1['x']}, {elem1['y']}) - {elem1['content']}")
            print(f"     Bounding box: ({bb1[0]}, {bb1[1]}) to ({bb1[2]}, {bb1[3]})")
            print(f"  2: {elem2['type']} at ({elem2['x']}, {elem2['y']}) - {elem2['content']}")
            print(f"     Bounding box: ({bb2[0]}, {bb2[1]}) to ({bb2[2]}, {bb2[3]})")
            print()
    else:
        print("\nNo overlapping elements found.")

if __name__ == "__main__":
    file_path = "check.pd"
    main(file_path)