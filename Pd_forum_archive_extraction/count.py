import os

def count_files_by_extension(folder_path):
    extension_counts = {}


    for root, _, files in os.walk(folder_path):
        for file_name in files:

            _, file_extension = os.path.splitext(file_name)

            file_extension = file_extension.lower()

            if file_extension in extension_counts:
                extension_counts[file_extension] += 1
            else:
                extension_counts[file_extension] = 1

    return extension_counts

def main():
    folder_path = 'Attachments-1720510204929' 
    extension_counts = count_files_by_extension(folder_path)


    print("File extensions and their counts in folder:")
    for extension, count in extension_counts.items():
        print(f"{extension}: {count}")

if __name__ == "__main__":
    main()
