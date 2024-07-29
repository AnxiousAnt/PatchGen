import csv
import re

def load_valid_objects(csv_file):
    valid_objects = set()
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            valid_objects.add(row['Object Name'].strip())
    # Add common Pd objects that might not be in the CSV
    valid_objects.update(['trigger', 'bang', 'float', 'symbol', 'list', 'bng', '+', '-', '*', '/',
                          'list append', 'list prepend', 'list split', 'list trim', 'list length'])
    return valid_objects

def validate_pd_patch(pd_file, valid_objects):
    invalid_objects = set()
    with open(pd_file, 'r') as f:
        content = f.read()
        # Updated regex to match Pd objects, including multi-word objects
        object_lines = re.findall(r'#X obj \d+ \d+ (.+?)(?:;|$)', content)
        for line in object_lines:
            # Split the line into words
            words = line.split()
            # Check progressively longer combinations of words
            for i in range(1, len(words) + 1):
                obj = ' '.join(words[:i])
                if obj in valid_objects:
                    break  # Found a valid object, stop checking
            else:
                # If no break occurred, no valid object was found
                invalid_objects.add(words[0])  # Add the first word as invalid
    return invalid_objects

def main():
    pd_file = 'test.pd'  
    csv_file = 'pd-objects-csv.csv'  

    valid_objects = load_valid_objects(csv_file)
    invalid_objects = validate_pd_patch(pd_file, valid_objects)
    
    if invalid_objects:
        print(f"The following objects in the Pd patch are invalid:")
        for obj in invalid_objects:
            print(f"- {obj}")
    else:
        print("All objects in the Pd patch are valid.")

if __name__ == "__main__":
    main()