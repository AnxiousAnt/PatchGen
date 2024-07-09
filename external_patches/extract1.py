import os
import csv

def extract_pd_files_to_csv(root_folder, output_csv):
    # Initialize a list to hold the rows of data
    data_rows = []
    
    # Initialize a serial number counter
    serial_no = 1
    
    # Walk through the directory
    for dirpath, _, filenames in os.walk(root_folder):
        for filename in filenames:
            if filename.endswith('.pd'):
                # Construct the full file path
                file_path = os.path.join(dirpath, filename)
                
                try:
                    # Read the content of the .pd file with UTF-8 encoding and replace errors
                    with open(file_path, 'r', encoding='utf-8', errors='replace') as file:
                        content = file.read()
                        
                        # Count the number of records (lines ending with ';')
                        num_records = sum(1 for line in content.splitlines() if line.strip().endswith(';'))
                except Exception as e:
                    # Print any other exceptions
                    print(f"Skipping file due to unexpected error: {file_path} - {e}")
                    continue
                
                # Construct the relative path
                relative_path = os.path.relpath(file_path, root_folder)
                
                # Append the data to the list
                data_rows.append([serial_no, filename, relative_path, content, num_records])
                
                # Increment the serial number
                serial_no += 1
    
    # Write the data to a CSV file
    with open(output_csv, 'w', newline='', encoding='utf-8') as csvfile:
        csvwriter = csv.writer(csvfile)
        
        # Write the header
        csvwriter.writerow(['Serial No.', 'File Name', 'Relative Path', 'PatchFile', 'Number of Records'])
        
        # Write the data rows
        csvwriter.writerows(data_rows)

# Define the root folder and output CSV file
root_folder = 'purr-data'
output_csv = 'output.csv'

# Run the function
extract_pd_files_to_csv(root_folder, output_csv)
