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
                    content = parts[4]
                    print(content)
                    if content == 'bng':
                        # Special case for bang object
                        width = height = int(parts[5])  # Size is the 5th parameter for bng
                    else:
                        # For other objects and messages
                        content = ' '.join(parts[4:])
                        width = max(len(content) * CHAR_WIDTH, CHAR_WIDTH)  # Ensure minimum width
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
    #print(elements)
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

def get_bounding_box(elem):
    return (elem['x'], elem['y'], elem['x'] + elem['width'], elem['y'] + elem['height'])

def main(file_path):
    elements = parse_pd_file(file_path)
    overlaps = find_overlaps(elements)
    
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
    file_path = "counter.pd"
    main(file_path)