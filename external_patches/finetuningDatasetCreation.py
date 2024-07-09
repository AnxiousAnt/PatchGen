import pandas as pd

def filter_and_save_csv(input_csv, output_csv, threshold):
    df = pd.read_csv(input_csv)
    
    df_filtered = df[df['Number of Records'] <= threshold]

    df_filtered = df_filtered.drop(['Serial No.'], axis = 1)
    
    df_filtered.to_csv(output_csv, index=False)
    
    print(f"Filtered data saved to '{output_csv}'.")


#filter_and_save_csv('output_all_clean_patches_dropna.csv', 'output_filtered.csv', 40)


def extend():
    file_path = 'output_filtered.csv'  
    df = pd.read_csv(file_path)

    df_duplicated = pd.concat([df] * 3, ignore_index=True)

    df_extended = pd.concat([df, df_duplicated], ignore_index=True)

    df_shuffled = df_extended.sample(frac=1).reset_index(drop=True)

    output_file_path = 'shuffled_data.csv'  

    df_shuffled.to_csv(output_file_path, index=False)

    print("Rows duplicated, shuffled, and saved to", output_file_path)

extend()