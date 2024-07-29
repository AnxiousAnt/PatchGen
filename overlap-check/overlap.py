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
    objects = []
    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith('#X obj'):
                parts = line.split()
                if len(parts) >= 5:
                    x, y = int(parts[2]), int(parts[3])
                    width, height = estimate_object_size(parts[4:])
                    objects.append({
                        'x': x, 'y': y,
                        'width': width, 'height': height,
                        'line': line.strip()
                    })
    return objects

def estimate_object_size(obj_parts):
    obj_text = ' '.join(obj_parts)
    fontsize = 10  # Default font size in Pd
    char_width, char_height = get_font_metrics(fontsize)
    
    # Estimate width based on text length
    width = len(obj_text) * char_width
    
    # Minimum widths for common objects
    min_widths = {
        'bang': 40,
        'toggle': 40,
        'slider': 30,
        'nbx': 60,
        'vsl': 30,
        'hsl': 140,
        'vradio': 40,
        'hradio': 140,
        'vu': 40,
    }
    
    # Check if it's a known object type and use minimum width if larger
    for obj_type, min_width in min_widths.items():
        if obj_text.startswith(obj_type):
            width = max(width, min_width)
            break
    
    # Height is typically standard unless it's a multi-line object
    height = char_height + 8 # Add some padding
    
    return width, height

def check_overlap(obj1, obj2):
    return (obj1['x'] < obj2['x'] + obj2['width'] and
            obj1['x'] + obj1['width'] > obj2['x'] and
            obj1['y'] < obj2['y'] + obj2['height'] and
            obj1['y'] + obj1['height'] > obj2['y'])

def find_overlaps(objects):
    overlaps = []
    for i, obj1 in enumerate(objects):
        for j, obj2 in enumerate(objects[i+1:], i+1):
            if check_overlap(obj1, obj2):
                overlaps.append((obj1, obj2))
    return overlaps

def main(file_path):
    objects = parse_pd_file(file_path)
    overlaps = find_overlaps(objects)
    
    if overlaps:
        print(f"Found {len(overlaps)} overlapping object pairs:")
        for obj1, obj2 in overlaps:
            print(f"Overlap detected between:")
            print(f"  1: {obj1['line']}")
            print(f"  2: {obj2['line']}")
            print()
    else:
        print("No overlapping objects found.")

if __name__ == "__main__":
    file_path = "overlap-check.pd"
    main(file_path)