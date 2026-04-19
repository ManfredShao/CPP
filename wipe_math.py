import re

with open('projects/report.md', 'r', encoding='utf-8') as f:
    text = f.read()

# Replace specific LaTeX commands
text = text.replace(r'\le', '<=')
text = text.replace(r'\ge', '>=')

# Find all $...$ and replace with `...`
text = re.sub(r'\$(.*?)\$', r'`\1`', text)

with open('projects/report.md', 'w', encoding='utf-8') as f:
    f.write(text)
