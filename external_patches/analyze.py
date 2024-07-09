import pandas as pd



input_file = 'output_filtered.csv'

# Read the CSV file
df = pd.read_csv(input_file)

# Add an index column at the beginning
df.insert(0, 'Index', range(1, len(df) + 1))

# Replace 'output.csv' with your desired output file name
output_file = 'output_filtered_indexed.csv'

df.to_csv(output_file, index=False)

print(f"Index column added successfully. Output saved to '{output_file}'.")
