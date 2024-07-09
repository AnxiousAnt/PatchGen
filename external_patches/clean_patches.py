import re

def remove_subpatches(input_file, output_file):
    with open(input_file, 'r') as file:
        lines = file.readlines()

    # Initialize variables to keep track of sub-patch state
    in_subpatch = False
    subpatch_level = 0
    cleaned_lines = []

    subpatch_start_pattern = re.compile(r'^#N canvas \d+ \d+ \d+ \d+ \S+ \d+;$')
    subpatch_end_pattern = re.compile(r'^#X restore \d+ \d+ pd \S+;$')

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
        
        # If not in a sub-patch, add line to the cleaned lines
        if not in_subpatch:
            cleaned_lines.append(line)

    # Write the cleaned lines to the output file
    with open(output_file, 'w') as file:
        file.writelines(cleaned_lines)

# Example usage
remove_subpatches('input.pd', 'output.pd')
