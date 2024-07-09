import os
import csv

def extract_pd_files_to_csv(root_folder, output_csv):
    data_rows = []
    
    serial_no = 1
    
    # Walk through the directory
    for dirpath, _, filenames in os.walk(root_folder):
        for filename in filenames:
            if filename.endswith('.pd'):
                file_path = os.path.join(dirpath, filename)
                
                try:
                    # Read the content of the .pd file with UTF-8 encoding
                    with open(file_path, 'r', encoding='utf-8') as file:
                        content = file.read()
                except UnicodeDecodeError:
                    # Skip files that can't be decoded
                    print(f"Skipping file due to encoding error: {file_path}")
                    continue
                
                relative_path = os.path.relpath(file_path, root_folder)
                
                data_rows.append([serial_no, filename, relative_path, content])
                
                serial_no += 1
    
    with open(output_csv, 'w', newline='', encoding='utf-8') as csvfile:
        csvwriter = csv.writer(csvfile)
        
        csvwriter.writerow(['Serial No.', 'File Name', 'Relative Path', 'Content'])
        
        csvwriter.writerows(data_rows)

root_folder = 'externals'
output_csv = 'output1.csv'

extract_pd_files_to_csv(root_folder, output_csv)
