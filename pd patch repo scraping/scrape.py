import csv
import json
from bs4 import BeautifulSoup

def extract_and_save_data(file_path, output_csv_path):
    try:
        # Load the HTML content from the file
        with open(file_path, 'r', encoding='utf-8') as file:
            html_content = file.read()
        
        # Parse the HTML content using BeautifulSoup
        soup = BeautifulSoup(html_content, 'html.parser')
        
        # Extract the topic title
        try:
            topic_title = soup.find('span', {'component': 'topic/title'}).text
        except AttributeError:
            raise ValueError("Topic title not found in the HTML content.")
        
        # Extract the topic text from the first occurrence of <div class="topic-text">
        try:
            topic_text_div = soup.find('div', class_='topic-text')
            topic_text = topic_text_div.find('div', {'component': 'post/content'}).text.strip()
        except AttributeError:
            raise ValueError("Topic text not found in the HTML content.")
        
        # Find all 'a' tags with href ending in '.pd' or '.zip' and create a JSON object
        links = []
        try:
            for a_tag in topic_text_div.find_all('a', href=True):
                if a_tag['href'].endswith('.pd') or a_tag['href'].endswith('.zip'):
                    links.append({'text': a_tag.text, 'link': a_tag['href']})
        except Exception as e:
            raise ValueError(f"Error occurred while extracting links: {e}")
        
        # Prepare data to write to CSV
        csv_data = {
            'topic_title': topic_title,
            'topic_text': topic_text,
            'links': json.dumps(links)
        }
        
        # Write the data to CSV
        with open(output_csv_path, 'w', newline='', encoding='utf-8') as csvfile:
            fieldnames = ['topic_title', 'topic_text', 'links']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerow(csv_data)
        
        print(f"Data successfully extracted and saved to {output_csv_path}")
    
    except FileNotFoundError:
        print(f"File not found: {file_path}")
    except IOError:
        print(f"Error reading file: {file_path}")
    except Exception as e:
        print(f"An error occurred: {e}")

# Define the file paths
input_file_path = 'Map Object _ PURE DATA forum~.html'  # Replace with your actual file path
output_csv_path = 'output.csv'  # Replace with your desired output file path

# Extract data and save to CSV
extract_and_save_data(input_file_path, output_csv_path)
