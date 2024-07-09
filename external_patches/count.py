import os

def count_pd_files(directory):
    pd_files = set()
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".pd"):
                full_path = os.path.join(root, file)
                pd_files.add(full_path)
    return pd_files

# Example usage
directory = "doc"
pd_file_count = count_pd_files(directory)
print(pd_file_count)
print(f"Total number of unique .pd files: {len(pd_file_count)}")
