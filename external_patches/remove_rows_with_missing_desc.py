import pandas as pd

input_file = 'output_all_clean_patches.csv' 
output_file = 'output_all_clean_patches_dropna.csv'  

df = pd.read_csv(input_file)

cleaned_df = df.dropna()

cleaned_df.to_csv(output_file, index=False)

print(f"Cleaned data saved to '{output_file}'.")
