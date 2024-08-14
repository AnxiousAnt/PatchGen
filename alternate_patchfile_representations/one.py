import re

def extract_records_from_pd_file(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    record_pattern = re.compile(r'#[^;]+;', re.DOTALL)
    records = record_pattern.findall(content)
    records = [' '.join(record.split()) for record in records]

    canvas_record = None
    object_records = []
    connection_records = []

    for record in records:
        if record.startswith('#N canvas'):
            canvas_record = record
        elif record.startswith('#X connect'):
            connection_records.append(record)
        else:
            object_records.append(record)

    return canvas_record, object_records, connection_records

def index_and_prepare_for_sorting(object_records):
    indexed_objects = []
    position_map = {}

    for i, record in enumerate(object_records):
        parts = record.split()
        print("\n\nparts: ", parts)
        if len(parts) >= 4 and parts[2].isdigit() and parts[3].isdigit():
            x, y = float(parts[2]), float(parts[3])
            print('\n\nx, y', x, y)
            indexed_objects.append((i, x, y, record))
        
        position_map[i] = i  # Initially, old position = new position

    print(indexed_objects, position_map)
    return indexed_objects, position_map

def sort_object_records(indexed_objects, position_map):
    # Sort based on x and y coordinates
    sorted_objects = sorted(indexed_objects, key=lambda obj: (obj[2], obj[1]))
    
    # Update position map
    for new_pos, (old_pos, _, _, _) in enumerate(sorted_objects):
        position_map[old_pos] = new_pos

    return [record for _, _, _, record in sorted_objects], position_map

# Example usage:
file_path = 'add-two.pd'
canvas, objects, connections = extract_records_from_pd_file(file_path)

indexed_objects, position_map = index_and_prepare_for_sorting(objects)
sorted_objects, updated_position_map = sort_object_records(indexed_objects, position_map)

print("Original object records:")
for i, obj in enumerate(objects):
    print(f"{i}: {obj}")

print("\nSorted object records:")
for i, obj in enumerate(sorted_objects):
    print(f"{i}: {obj}")

print("\nPosition mapping (old position -> new position):")
for old_pos, new_pos in updated_position_map.items():
    print(f"{old_pos} -> {new_pos}")