#!/bin/bash
find . -name "*.md" -type f | while read file; do
    echo "# File: $file" >> combined.txt
    echo "" >> combined.txt
    cat "$file" >> combined.txt
    echo "" >> combined.txt
done