import pandas as pd
from bs4 import BeautifulSoup

# Read the HTML content from a file
with open('test.html', 'r', encoding='utf-8') as file:
    html_content = file.read()

# Parse the HTML content
soup = BeautifulSoup(html_content, 'html.parser')

# Find the container with id="topics-container"
topics_container = soup.find('ul', id='topics-container')

# Extract all list items within the container
list_items = topics_container.find_all('li', class_='category-item')

# Prepare a list to store the extracted information
data = []

# Loop through each list item to extract the required information
for item in list_items:
    try:
        index = item.get('data-index')
        topic_header = item.find('a', {'component': 'topic/header'})
        
        if topic_header:
            title = topic_header.text.strip()
            link = topic_header['href']
            
            # Append the information to the data list
            data.append([index, title, link])
            
            # Print a confirmation message
            print(f'Row {index}: Title="{title}", Link="{link}"')
    except UnicodeEncodeError as e:
        print(f'Error processing row {index}: {e}')
        continue

# Create a DataFrame from the data
df = pd.DataFrame(data, columns=['Index', 'Title', 'Link'])

# Save the DataFrame to a CSV file with UTF-8 encoding
df.to_csv('extracted_topics.csv', index=False, encoding='utf-8')

print('All rows have been processed and saved to extracted_topics.csv.')
