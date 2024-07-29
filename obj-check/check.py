import re
import csv
from html import unescape

def extract_pd_objects(content):
    # Regular expression to match object name and description
    pattern = r'<a class="pdobj" href="[^"]*">([^<]+)</a>\s*-\s*([^<]+)'
    
    # Find all matches
    matches = re.findall(pattern, content)
    
    # Unescape HTML entities and strip whitespace
    objects = [(unescape(name.strip()), unescape(desc.strip())) for name, desc in matches]
    
    return objects

def save_to_csv(objects, filename):
    with open(filename, 'w', newline='', encoding='utf-8') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['Object Name', 'Description'])  # Header
        writer.writerows(objects)

def read_mhtml_file(filename):
    with open(filename, 'r', encoding='utf-8') as file:
        return file.read()

def main():
    # Specify the input .mhtml file path here
    input_filename = 'List of objects _ Pure Data.mhtml'
    output_filename = 'pure_data_objects.csv'

    try:
        # Read the MHTML file
        content = read_mhtml_file(input_filename)

        # Extract objects
        pd_objects = extract_pd_objects(content)

        # Save to CSV
        save_to_csv(pd_objects, output_filename)

        print(f"Extracted {len(pd_objects)} Pure Data objects from {input_filename}")
        print(f"Saved to {output_filename}")

    except FileNotFoundError:
        print(f"Error: The file {input_filename} was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()