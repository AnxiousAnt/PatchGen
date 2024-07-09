import csv
import json
import requests
from bs4 import BeautifulSoup

def extract_data_from_html(html_content):
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

    return {
        'topic_title': topic_title,
        'topic_text': topic_text,
        'links': json.dumps(links)
    }

def extract_and_save_data_from_links(input_csv_path, output_csv_path):
    try:
        # Read the input CSV file
        with open(input_csv_path, 'r', encoding='utf-8') as csvfile:
            reader = csv.DictReader(csvfile)
            rows = list(reader)

        # Prepare to write the output CSV file
        with open(output_csv_path, 'w', newline='', encoding='utf-8') as csvfile:
            fieldnames = ['Index', 'Title', 'topic_title', 'topic_text', 'links']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()

            # Process each link
            for row in rows:
                index = row['Index']
                title = row['Title']
                link = row['Link']

                try:
                    # Get the HTML content from the link
                    response = requests.get(link)
                    response.raise_for_status()  # Check for request errors
                    html_content = response.text

                    # Extract data from the HTML content
                    data = extract_data_from_html(html_content)

                    # Prepare the row for the output CSV
                    csv_data = {
                        'Index': index,
                        'Title': title,
                        'topic_title': data['topic_title'],
                        'topic_text': data['topic_text'],
                        'links': data['links']
                    }

                    # Write the row to the output CSV
                    writer.writerow(csv_data)

                    # Print confirmation message
                    print(f"Successfully scraped data from {link}")

                except requests.RequestException as e:
                    print(f"Request error for {link}: {e}")
                except ValueError as e:
                    print(f"Data extraction error for {link}: {e}")
                except Exception as e:
                    print(f"An unexpected error occurred for {link}: {e}")

    except FileNotFoundError:
        print(f"File not found: {input_csv_path}")
    except IOError:
        print(f"Error reading file: {input_csv_path}")
    except Exception as e:
        print(f"An error occurred: {e}")

# Define the file paths
input_csv_path = 'modified_links_file.csv'  # Replace with your actual file path
output_csv_path = 'final.csv'  # Replace with your desired output file path

# Extract data and save to CSV
extract_and_save_data_from_links(input_csv_path, output_csv_path)
