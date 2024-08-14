import re

def extract_records_from_pd_file(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    # Use regex to extract records
    record_pattern = re.compile(r'#[^;]+;', re.DOTALL)
    records = record_pattern.findall(content)
    print('\n\nrecords: ', records)
    # Clean up whitespace in records
    records = [' '.join(record.split()) for record in records]
    print('\n\nrecords: ', records)

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

file_path = 'add-two.pd'
canvas, objects, connections = extract_records_from_pd_file(file_path)

print("Canvas record:")
print(canvas)
print("\nObject records:")
for obj in objects:
    print(obj)
print("\nConnection records:")
for conn in connections:
    print(conn)