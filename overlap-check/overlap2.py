import re

# Font metrics (assuming default font size of 10)
CHAR_WIDTH = 7
CHAR_HEIGHT = 13

def parse_pd_file(file_path):
    elements = []
    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith('#X obj') or line.startswith('#X msg'):
                parts = line.split()
                if len(parts) >= 5:
                    element_type = 'obj' if line.startswith('#X obj') else 'msg'
                    x, y = int(parts[2]), int(parts[3])
                    content = ' '.join(parts[4:])
                    width = len(content) * CHAR_WIDTH
                    height = CHAR_HEIGHT
                    elements.append({
                        'type': element_type,
                        'x': x,
                        'y': y,
                        'width': width,
                        'height': height,
                        'content': content,
                        'line': line.strip()
                    })
    return elements

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

def main(file_path):
    elements = parse_pd_file(file_path)
    overlaps = find_overlaps(elements)
    
    if overlaps:
        print(f"Found {len(overlaps)} overlapping pairs:")
        for elem1, elem2 in overlaps:
            print(f"Overlap detected between:")
            print(f"  1: {elem1['type']} at ({elem1['x']}, {elem1['y']}) - {elem1['content']}")
            print(f"     Width: {elem1['width']}, Height: {elem1['height']}")
            print(f"  2: {elem2['type']} at ({elem2['x']}, {elem2['y']}) - {elem2['content']}")
            print(f"     Width: {elem2['width']}, Height: {elem2['height']}")
            print()
    else:
        print("No overlapping elements found.")

if __name__ == "__main__":
    file_path = "overlap-check.pd"
    main(file_path)