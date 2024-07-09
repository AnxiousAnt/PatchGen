from bs4 import BeautifulSoup

# Read the HTML file
with open('PdFileFormat — Pd Community Site.html', 'r', encoding='utf-8') as file:
    html_content = file.read()

# Parse the HTML content
soup = BeautifulSoup(html_content, 'html.parser')

# Find all anchor tags with class 'new visualNoPrint'
for tag in soup.find_all('a', class_='new visualNoPrint'):
    tag.decompose()  # Remove the tag from the soup

# Save the modified HTML back to a file
with open('Modified_PdFileFormat.html', 'w', encoding='utf-8') as file:
    file.write(str(soup))

print("Anchor tags with class 'new visualNoPrint' have been removed.")
