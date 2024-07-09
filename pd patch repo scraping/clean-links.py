import pandas as pd

# Step 1: Read the CSV file into a DataFrame
df = pd.read_csv('extracted_topics.csv')

# Step 2: Filter out rows where 'Title' contains 'This topic is deleted!'
df = df[~df['Title'].str.contains('This topic is deleted!')]

# Step 3: Reset index to make it consistent after filtering
df = df.reset_index(drop=True)

# Step 4: Modify the 'Link' column to prepend the string if necessary
df['Link'] = df['Link'].apply(lambda x: 'https://forum.pdpatchrepo.info' + x if not x.startswith('https://forum.pdpatchrepo.info') else x)

# Step 5: Drop the existing 'Index' column
df = df.drop(columns=['Index'])

# Step 6: Create a new column 'new-index' to store row numbers
df['new-index'] = df.index + 1  # Adding 1 to start index from 1 if needed

# Step 7: Save the modified DataFrame to a new CSV file
df.to_csv('modified_links_file.csv', index=False)

print('Modified CSV file saved successfully.')

