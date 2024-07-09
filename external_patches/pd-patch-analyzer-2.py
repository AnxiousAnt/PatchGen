import csv
import json
import re

def extract_objects(patch):
    # Regular expression to match Pd objects
    object_pattern = r'#X obj.*?\s(\S+);'
    return re.findall(object_pattern, patch)

def analyze_patches(csv_file):
    object_data = {}

    with open(csv_file, 'r', newline='', encoding='utf-8') as file:
        csv_reader = csv.DictReader(file)
        for row in csv_reader:
            patch = row['Cleaned Patches']
            index = row['Index']
            objects = extract_objects(patch)
            
            # Count unique objects in this patch and store index
            unique_objects = set(objects)
            for obj in unique_objects:
                if obj not in object_data:
                    object_data[obj] = {
                        'count': 1,
                        'indexes': [index]
                    }
                else:
                    object_data[obj]['count'] += 1
                    object_data[obj]['indexes'].append(index)

    return object_data

def save_to_json(data, output_file):
    with open(output_file, 'w') as f:
        json.dump(data, f, indent=2)

# Main execution
csv_file = 'output_filtered_indexed.csv'  # Replace with your CSV file path
output_file = 'pd_object_data_2.json'

object_data = analyze_patches(csv_file)
save_to_json(object_data, output_file)

print(f"Analysis complete. Results saved to {output_file}")