import csv
import json
import re

def extract_objects(patch):
   
    object_pattern = r'#X obj.*?\s(\S+);'
    return re.findall(object_pattern, patch)

def analyze_patches(csv_file):
    object_counts = {}
    patch_counts = {}

    with open(csv_file, 'r', newline='', encoding='utf-8') as file:
        csv_reader = csv.DictReader(file)
        for row in csv_reader:
            patch = row['Cleaned Patches']
            objects = extract_objects(patch)
            
            # Count unique objects in this patch
            unique_objects = set(objects)
            for obj in unique_objects:
                if obj not in object_counts:
                    object_counts[obj] = 1
                    patch_counts[obj] = 1
                else:
                    object_counts[obj] += 1
                    patch_counts[obj] += 1

    return patch_counts

def save_to_json(data, output_file):
    with open(output_file, 'w') as f:
        json.dump(data, f, indent=2)


csv_file = 'output_filtered.csv'  
output_file = 'pd_object_counts.json'

object_patch_counts = analyze_patches(csv_file)
save_to_json(object_patch_counts, output_file)

print(f"Analysis complete. Results saved to {output_file}")
