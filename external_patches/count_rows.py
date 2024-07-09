import pandas as pd

def count_csv_rows(csv_file):
    try:
        df = pd.read_csv(csv_file, encoding='utf-8-sig')
        row_count = len(df)
        return row_count
    except IOError as e:
        print(f"Error reading the file: {e}")
    except pd.errors.ParserError as e:
        print(f"Error parsing CSV: {e}")
    except UnicodeDecodeError as e:
        print(f"Unicode decoding error: {e}")

# Example usage:
csv_file = 'output-all.csv'
num_rows = count_csv_rows(csv_file)
if num_rows is not None:
    print(f"Number of rows in '{csv_file}': {num_rows}")
