import csv
import re

def load_valid_objects(csv_file):
    valid_objects = set()
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            valid_objects.add(row['Object Name'].strip())
    return valid_objects

def validate_pd_patch(pd_file, valid_objects):
    invalid_objects = set()
    with open(pd_file, 'r') as f:
        content = f.read()
        objects = re.findall(r'#X obj \d+ \d+ (\S+)(?:;|\s|$)', content)
        for obj in objects:
            # Remove trailing semicolon if present
            obj = obj.rstrip(';')
            if obj not in valid_objects:
                invalid_objects.add(obj)
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