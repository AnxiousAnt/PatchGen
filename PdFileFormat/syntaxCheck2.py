def read_pd_file(file_path):
    """
    Reads a .pd file and splits it into individual records.
    
    Parameters:
    - file_path (str): Path to the .pd file.
    
    Returns:
    - records (list): List of records, where each record is a dictionary with 
      'chunk_type', 'element_type', 'parameters', and 'original_line'.
    """
    records = []
    
    with open(file_path, 'r') as file:
        for i, line in enumerate(file):
            # Strip the line endings
            line = line.strip()
            
            # Skip empty lines
            if not line:
                continue
            
            # Check if the line ends with a semicolon and starts with '#'
            if not line.endswith(';') or not line.startswith('#'):
                records.append({
                    'chunk_type': None,
                    'element_type': None,
                    'parameters': None,
                    'original_line': line,
                    'index': i,
                    'error': f"Syntax error: Line does not start with '#' or end with ';': {line}"
                })
                continue
            
            # Remove the leading '#' and trailing ';'
            line_content = line[1:-1].strip()
            
            # Split the line into parts
            parts = line_content.split()
            
            if len(parts) < 2:
                records.append({
                    'chunk_type': None,
                    'element_type': None,
                    'parameters': None,
                    'original_line': line,
                    'index': i,
                    'error': f"Syntax error: Insufficient parts in the line: {line}"
                })
                continue
            
            # Extract chunk type and element type
            chunk_type = parts[0]
            element_type = parts[1]
            parameters = parts[2:]
            
            # Append to the records list
            records.append({
                'chunk_type': chunk_type,
                'element_type': element_type,
                'parameters': parameters,
                'original_line': line,
                'index': i,
                'error': None  # No error initially
            })
    
    return records

def validate_canvas(parameters):
    errors = []
    if len(parameters) != 6:
        errors.append("Canvas element must have exactly 6 parameters.")
    try:
        int(parameters[0])
        int(parameters[1])
        int(parameters[2])
        int(parameters[3])
    except ValueError:
        errors.append("Canvas positions and sizes must be integers.")
    try:
        int(parameters[5])
    except ValueError:
        errors.append("Canvas 'open_on_load' flag must be an integer (0 or 1).")
    return errors

def validate_connect(parameters):
    errors = []
    if len(parameters) != 4:
        errors.append("Connect element must have exactly 4 parameters.")
    try:
        int(parameters[0])
        int(parameters[1])
        int(parameters[2])
        int(parameters[3])
    except ValueError:
        errors.append("Connect parameters must be integers.")
    return errors

def validate_obj(parameters):
    errors = []
    if len(parameters) < 3:
        errors.append("Object element must have at least 3 parameters.")
    return errors

def validate_msg(parameters):
    errors = []
    if len(parameters) < 2:
        errors.append("Message element must have at least 2 parameters.")
    return errors

def validate_pd_records(records):
    errors = []
    for record in records:
        if record['error'] is not None:
            errors.append(record)
            continue
        chunk_type = record['chunk_type']
        element_type = record['element_type']
        parameters = record['parameters']
        
        if chunk_type not in {"X", "N", "A"}:
            record['error'] = f"Invalid chunk type '{chunk_type}'."
            errors.append(record)
            continue
        
        if element_type == "canvas":
            canvas_errors = validate_canvas(parameters)
            if canvas_errors:
                record['error'] = "; ".join(canvas_errors)
                errors.append(record)
        elif element_type == "connect":
            connect_errors = validate_connect(parameters)
            if connect_errors:
                record['error'] = "; ".join(connect_errors)
                errors.append(record)
        elif element_type == "obj":
            obj_errors = validate_obj(parameters)
            if obj_errors:
                record['error'] = "; ".join(obj_errors)
                errors.append(record)
        elif element_type == "msg":
            msg_errors = validate_msg(parameters)
            if msg_errors:
                record['error'] = "; ".join(msg_errors)
                errors.append(record)
        else:
            # Add validation for other element types as needed
            pass
    
    return errors

def display_with_errors(file_path, records, errors):
    with open(file_path, 'r') as file:
        original_lines = file.readlines()
    
    error_indices = {record['index']: record['error'] for record in errors}
    
    print("Original Patch:")
    for i, line in enumerate(original_lines):
        print(f"{i}: {line.strip()}")
    
    print("\nPatch with Errors Highlighted:")
    for i, record in enumerate(records):
        print(f"{record['index']}: {record['original_line'].strip()}")
        if record['index'] in error_indices:
            print(f"  >> Error: {error_indices[record['index']]}")
    
# Writing the sample file content to 'sample.pd'
sample_pd_content = """
#N canvas 0 0 450 300 sample_patch 1;
#N canvas taes err
#X obj 50 50 osc~ 440;
#X obj 50 100 dac~;
#X connect 0 0 1 0;
#X connect 0 1 1;
"""

with open('sample.pd', 'w') as file:
    file.write(sample_pd_content.strip())

# Reading and validating the sample file
file_path = 'sample.pd'
records = read_pd_file(file_path)
errors = validate_pd_records(records)

if errors:
    display_with_errors(file_path, records, errors)
else:
    print("No syntax errors found.")
