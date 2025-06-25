import os

def count_lines(path):
    lines = 0
    for item in os.listdir(path):
        subpath = os.path.join(path, item)
        if os.path.isdir(subpath):
            lines += count_lines(subpath)
        elif os.path.isfile(subpath):
            file_name, file_extension = os.path.splitext(subpath)
            if file_extension in ['.h', '.cpp', '.bat', '.glsl', '.rc', '.py']:
                with open(subpath) as f:
                    lines += len(f.readlines())
    return lines

total_lines = 0
print("")
for path in ["bat", "GameLibrary", "GameAssets", "Win32PlatformLayer", "scripts"]:
    lines = count_lines(path)
    total_lines += lines
    print(f"{path}: {lines}")

print(f"Total: {total_lines}")