import pandas as pd
import matplotlib.pyplot as plt
from collections import Counter
import logging
import time

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# Start script
logging.info('Script started')

start_time = time.time()

# Read the CSV files
logging.info('Reading CSV files...')
patches_df = pd.read_csv('histogram-plot/appended_output.csv')
objects_df = pd.read_csv('histogram-plot/pd-objects-csv.csv')

# Get the list of object names
object_names = objects_df['Object Name'].tolist()

# Initialize a counter for the object occurrences
object_counter = Counter()

# Ensure 'Patch Contents' column is of string type and handle NaNs
logging.info('Processing patch contents...')
patches_df['Patch Contents'] = patches_df['Patch Contents'].fillna('').astype(str)

# Loop through the patch contents and count occurrences of each object
for i, patch_content in enumerate(patches_df['Patch Contents']):
    if i % 1000 == 0:  # Log progress every 1000 rows
        logging.info(f'Processing row {i}/{len(patches_df)}')
    for object_name in object_names:
        object_counter[object_name] += patch_content.split().count(object_name)

# Save the results to a text file
logging.info('Saving results to object_counts_all.txt...')
with open('object_counts_all.txt', 'w') as file:
    for object_name, count in object_counter.most_common():
        file.write(f"{object_name}: {count}\n")

num_objects_to_plot = 20

# Get the most common objects
logging.info('Generating plot for the most common objects...')
most_common_objects = object_counter.most_common(num_objects_to_plot)

# Plot a histogram of the object occurrences
plt.figure(figsize=(10, 6))
plt.bar([obj[0] for obj in most_common_objects], [obj[1] for obj in most_common_objects], color='skyblue')
plt.xlabel('Object Name')
plt.ylabel('Occurrences')
plt.title(f'Top {num_objects_to_plot} Most Used Objects in Pd Patches')
plt.xticks(rotation=90)
plt.tight_layout()

# Save the plot as an image
plt.savefig('object_histogram_all.png')
logging.info('Plot saved as object_histogram_all.png')

# Show the plot
plt.show()

# End script
end_time = time.time()
elapsed_time = end_time - start_time
logging.info(f'Script finished in {elapsed_time:.2f} seconds')
