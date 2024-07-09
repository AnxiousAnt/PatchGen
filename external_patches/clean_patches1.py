import re

def remove_subpatches(lines):
    """Remove sub-patches from the list of lines."""
    subpatch_start_pattern = re.compile(r'^#N canvas \d+ \d+ \d+ \d+ \S+ \d+;$')
    subpatch_end_pattern = re.compile(r'^#X restore \d+ \d+ pd \S+;$')
    
    in_subpatch = False
    subpatch_level = 0
    cleaned_lines = []

    for line in lines:
        # Detect start of a sub-patch
        if subpatch_start_pattern.match(line):
            if in_subpatch:
                subpatch_level += 1
            else:
                in_subpatch = True
                subpatch_level = 0
            continue
        
        # Detect end of a sub-patch
        if subpatch_end_pattern.match(line):
            if subpatch_level == 0:
                in_subpatch = False
            else:
                subpatch_level -= 1
            continue
        
        # If in a sub-patch, skip the line
        if in_subpatch:
            continue

        # Add line to the cleaned lines if it doesn't match any patterns to remove
        cleaned_lines.append(line)
    
    return cleaned_lines

def remove_cnv_pddp_objects(lines):
    """Remove cnv and pddp/pddplink objects from the list of lines."""
    cnv_pattern = re.compile(r'#X obj \d+ \d+ cnv .*')
    pddp_pattern = re.compile(r'#X obj \d+ \d+ pddp/pddplink .*')

    cleaned_lines = []
    current_object = []

    for line in lines:
        current_object.append(line)
        if line.strip().endswith(';'):
            full_object = ''.join(current_object)
            if not (cnv_pattern.match(full_object) or pddp_pattern.match(full_object)):
                cleaned_lines.extend(current_object)
            current_object = []

    return cleaned_lines

def process_pd_file(input_file, output_file):
    with open(input_file, 'r') as file:
        lines = file.readlines()

    # Remove sub-patches
    lines = remove_subpatches(lines)
    # Remove cnv and pddp/pddplink objects
    lines = remove_cnv_pddp_objects(lines)

    # Write the cleaned lines to the output file
    with open(output_file, 'w') as file:
        file.writelines(lines)

# Example usage
process_pd_file('input.pd', 'output1.pd')
