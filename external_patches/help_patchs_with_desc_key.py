import os
import csv

def extract_help_pd_files_to_csv(root_folder, output_csv):
    # Initialize a list to hold the rows of data
    data_rows = []
    
    # Initialize a serial number counter
    serial_no = 1
    
    # Walk through the directory
    for dirpath, _, filenames in os.walk(root_folder):
        for filename in filenames:
            if filename.lower().endswith('.pd') and 'help' in filename.lower():
                # Construct the full file path
                file_path = os.path.join(dirpath, filename)
                
                try:
                    # Read the content of the .pd file with UTF-8 encoding and replace errors
                    with open(file_path, 'r', encoding='utf-8', errors='replace') as file:
                        content = file.read()
                        
                        # Count the number of records (lines ending with ';')
                        num_records = sum(1 for line in content.splitlines() if line.strip().endswith(';'))
                        
                        # Initialize variables to extract keywords and description
                        keywords = "doesn't exist"
                        description = "doesn't exist"
                        
                        # Function to extract multiline content
                        def extract_multiline_content(start_keyword, text):
                            start_idx = text.find(start_keyword)
                            if start_idx == -1:
                                return "doesn't exist"
                            start_idx += len(start_keyword)
                            end_idx = text.find(';', start_idx)
                            if end_idx == -1:
                                end_idx = len(text)
                            return text[start_idx:end_idx].strip()
                        
                        keywords = extract_multiline_content('KEYWORDS', content)
                        description = extract_multiline_content('DESCRIPTION', content)
                        
                        # Include only files with both keywords and description
                        if keywords != "doesn't exist" and description != "doesn't exist":
                            # Construct the relative path
                            relative_path = os.path.relpath(file_path, root_folder)
                            
                            # Append the data to the list
                            data_rows.append([serial_no, filename, relative_path, content, num_records, keywords, description])
                            
                            # Increment the serial number
                            serial_no += 1
                
                except Exception as e:
                    # Print any other exceptions
                    print(f"Skipping file due to unexpected error: {file_path} - {e}")
                    continue
    
    # Write the data to a CSV file
    with open(output_csv, 'w', newline='', encoding='utf-8') as csvfile:
        csvwriter = csv.writer(csvfile)
        
        # Write the header
        csvwriter.writerow(['Serial No.', 'File Name', 'Relative Path', 'Content', 'Number of Records', 'Keywords', 'Description'])
        
        # Write the data rows
        csvwriter.writerows(data_rows)

# Define the root folder and output CSV file
root_folder = 'purr-data'
output_csv = 'output_help_files_desc_key_strict.csv'

# Run the function
extract_help_pd_files_to_csv(root_folder, output_csv)
