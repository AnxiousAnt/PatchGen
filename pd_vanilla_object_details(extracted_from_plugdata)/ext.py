import os
import json
import yaml
import re

def extract_yaml_and_content(file_path):
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()
    
    # Split the file into YAML front matter and Markdown content
    parts = re.split(r'^---\s*$', content, 2, re.MULTILINE)
    
    if len(parts) < 3:
        return None, content.strip()
    
    yaml_content = parts[1].strip()
    markdown_content = parts[2].strip()
    
    try:
        yaml_data = yaml.safe_load(yaml_content)
    except yaml.YAMLError:
        return None, content.strip()
    
    return yaml_data, markdown_content

def process_directory(directory):
    result = []
    
    for filename in os.listdir(directory):
        if filename.endswith('.md'):
            file_path = os.path.join(directory, filename)
            yaml_data, markdown_content = extract_yaml_and_content(file_path)
            
            if yaml_data is not None:
                yaml_data['content'] = markdown_content
                yaml_data['filename'] = filename
                result.append(yaml_data)
    
    return result

def main():
    directory = input("Enter the directory path containing .md files: ")
    output_file = input("Enter the output JSON file name: ")
    
    if not os.path.isdir(directory):
        print("Invalid directory path.")
        return
    
    data = process_directory(directory)
    
    with open(output_file, 'w', encoding='utf-8') as json_file:
        json.dump(data, json_file, indent=2, ensure_ascii=False)
    
    print(f"Conversion complete. JSON file saved as {output_file}")

if __name__ == "__main__":
    main()