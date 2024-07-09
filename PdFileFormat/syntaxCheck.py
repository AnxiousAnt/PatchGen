def read_pd_file(file_path):
    """
    Reads a .pd file and splits it into individual records.
    
    Parameters:
    - file_path (str): Path to the .pd file.
    
    Returns:
    - records (list): List of records, where each record is a dictionary with 
      'chunk_type', 'element_type', and 'parameters'.
    """
    records = []
    
    with open(file_path, 'r') as file:
        for line in file:
            # Strip the line endings
            line = line.strip()
            
            # Skip empty lines
            if not line:
                continue
            
            # Check if the line ends with a semicolon and starts with '#'
            if not line.endswith(';') or not line.startswith('#'):
                print(f"Syntax error: Line does not start with '#' or end with ';': {line}")
                continue
            
            # Remove the leading '#' and trailing ';'
            line = line[1:-1].strip()
            
            # Split the line into parts
            parts = line.split()
            
            if len(parts) < 2:
                print(f"Syntax error: Insufficient parts in the line: {line}")
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
                'original_line': line  # Store the original line for later display
            })
    
    return records

def validate_canvas(parameters):
    """
    Validates the parameters of a 'canvas' element.
    
    Parameters:
    - parameters (list): List of parameters for the 'canvas' element.
    
    Returns:
    - errors (list): List of error messages.
    """
    errors = []
    
    if len(parameters) != 6:
        errors.append(f"Canvas element must have exactly 6 parameters, found {len(parameters)}.")
        return errors
    
    try:
        x_pos = int(parameters[0])
        y_pos = int(parameters[1])
        x_size = int(parameters[2])
        y_size = int(parameters[3])
    except ValueError:
        errors.append("Canvas positions and sizes must be integers.")
    
    name = parameters[4]
    
    try:
        open_on_load = int(parameters[5])
        if open_on_load not in {0, 1}:
            errors.append("Canvas 'open_on_load' flag must be 0 or 1.")
    except ValueError:
        errors.append("Canvas 'open_on_load' flag must be an integer (0 or 1).")
    
    return errors

def validate_pd_records(records):
    """
    Validates the structure and parameters of the parsed .pd records.
    
    Parameters:
    - records (list): List of records parsed from a .pd file.
    
    Returns:
    - errors (list): List of error messages.
    """
    errors = []
    valid_chunk_types = {"X", "N", "A"}
    valid_element_types = {
        "array", "connect", "coords", "floatatom", "msg", "obj", "bng", "tgl",
        "nbx", "vsl", "hsl", "vradio", "hradio", "vu", "cnv", "pd", "restore",
        "symbolatom", "text", "canvas"
    }
    
    for i, record in enumerate(records):
        chunk_type = record['chunk_type']
        element_type = record['element_type']
        parameters = record['parameters']
        
        if chunk_type not in valid_chunk_types:
            errors.append(f"Record {i}: Invalid chunk type '{chunk_type}'.")
        
        if element_type not in valid_element_types:
            errors.append(f"Record {i}: Invalid element type '{element_type}'.")
        
        # Detailed validation for specific element types
        if element_type == "canvas":
            canvas_errors = validate_canvas(parameters)
            if canvas_errors:
                errors.extend([f"Record {i}: {error}" for error in canvas_errors])
    
    return errors

def display_with_errors(file_path, errors):
    """
    Displays the content of a .pd file with errors highlighted and explained.
    
    Parameters:
    - file_path (str): Path to the .pd file.
    - records (list): List of records parsed from a .pd file.
    - errors (list): List of error messages.
    """
    with open(file_path, 'r') as file:
        lines = file.readlines()
    
    error_indices = [int(error.split()[1].strip(":")) for error in errors]
    for i, line in enumerate(lines):
        print(line.strip())
        if i in error_indices:
            for error in errors:
                if int(error.split()[1].strip(":")) == i:
                    print(f"  >> Error: {error}")
    
# Writing the sample file content to 'sample.pd'
sample_pd_patch = """
#N canvas 0 0 450  sample_patch 1;
#X obj 50 50 osc~ 440;
hey!
#X obj 50 100 dac~;
#N canvas dfa;
#X connect 0 0 1 0;
#MDMF loll fadlkf:
"""

with open('sample.pd', 'w') as file:
    file.write(sample_pd_patch.strip())

# Reading and validating the sample file
file_path = 'sample.pd'
records = read_pd_file(file_path)
errors = validate_pd_records(records)

if errors:
    display_with_errors(file_path, errors)
else:
    print("No syntax errors found.")


print(errors)
