import re, os, glob

def remove_comments(code):
    code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)
    code = re.sub(r'//[^\n]*', '', code)
    code = re.sub(r'\n{3,}', '\n\n', code)
    return code

for path in (
    glob.glob('Thorns/include/**/*.h',  recursive=True) +
    glob.glob('Thorns/src/**/*.cpp',    recursive=True)
):
    # errors='ignore' drops any unreadable bytes safely
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    with open(path, 'w', encoding='utf-8') as f:
        f.write(remove_comments(content))
    print(f"Processed: {path}")

print("Done")