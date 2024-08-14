import re
import os
import sys

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
        if len(parts) >= 4 and parts[2].isdigit() and parts[3].isdigit():
            x, y = float(parts[2]), float(parts[3])
            indexed_objects.append((i, x, y, record))
        
        position_map[i] = i  # Initially, old position = new position
    return indexed_objects, position_map

def sort_object_records(indexed_objects, position_map):
    sorted_objects = sorted(indexed_objects, key=lambda obj: (obj[2], obj[1]))
    
    for new_pos, (old_pos, _, _, _) in enumerate(sorted_objects):
        position_map[old_pos] = new_pos
    return [record for _, _, _, record in sorted_objects], position_map

def update_connection_records(connection_records, position_map):
    updated_connections = []
    for connection in connection_records:
        parts = connection.split()
        if len(parts) >= 6:  # Ensure we have enough parts to update
            old_from = int(parts[2])
            old_to = int(parts[4])
            new_from = position_map.get(old_from, old_from)
            new_to = position_map.get(old_to, old_to)
            updated_connection = f"#X connect {new_from} {parts[3]} {new_to} {parts[5]}"
            updated_connections.append(updated_connection)
        else:
            updated_connections.append(connection)  # Keep unchanged if format is unexpected
    return updated_connections

def sort_connection_records(connection_records):
    def connection_key(connection):
        parts = connection.split()
        return (int(parts[2]), int(parts[4]), int(parts[3]))
    return sorted(connection_records, key=connection_key)

def write_sorted_patch(sorted_patch, output_file_path):
    with open(output_file_path, 'w') as file:
        for record in sorted_patch:
            file.write(record + '\n')

def process_pd_file(input_file_path, output_file_path):
    canvas, objects, connections = extract_records_from_pd_file(input_file_path)
    indexed_objects, position_map = index_and_prepare_for_sorting(objects)
    sorted_objects, updated_position_map = sort_object_records(indexed_objects, position_map)
    updated_connections = update_connection_records(connections, updated_position_map)
    sorted_connections = sort_connection_records(updated_connections)
    sorted_patch = [canvas] + sorted_objects + sorted_connections
    write_sorted_patch(sorted_patch, output_file_path)

def process_directory(input_dir, output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    for filename in os.listdir(input_dir):
        if filename.endswith('.pd'):
            input_file_path = os.path.join(input_dir, filename)
            output_file_path = os.path.join(output_dir, filename)
            process_pd_file(input_file_path, output_file_path)
            print(f"Processed: {filename}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py input_directory output_directory")
        sys.exit(1)
    
    input_directory = sys.argv[1]
    output_directory = sys.argv[2]
    
    process_directory(input_directory, output_directory)
    print("All .pd files have been processed and sorted.")