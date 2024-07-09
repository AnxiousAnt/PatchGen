import json

with open('sorted_output.json', 'r') as f:
    data = json.load(f)

# Initialize a counter for keys with 'count' value of 1
count_of_keys = 0

# Iterate over each key in the dictionary
for key in data:
    # Check if 'count' value is 1 for the current key
    if data[key]['count'] == 1:
        count_of_keys += 1

# Print the count of keys where 'count' value is 1
print("Number of keys where 'count' value is 1:", count_of_keys)
