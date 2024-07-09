import json

# Load JSON data from file
with open('pd_object_data_2.json', 'r') as file:
    data = json.load(file)

# Filter out keys that are numeric
filtered_data = {key: value for key, value in data.items() if not key.isdigit()}

# Sort by 'count' value in ascending order
sorted_data = dict(sorted(filtered_data.items(), key=lambda item: item[1]['count']))

# Convert sorted dictionary back to JSON
output_json = json.dumps(sorted_data, indent=2)

# Write output JSON to a new file
output_file = 'sorted_output.json'
with open(output_file, 'w') as file:
    file.write(output_json)

print(f'Output saved to {output_file}')
