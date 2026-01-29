#!/bin/bash

# --- Git History Analyzer ---
#
# This script analyzes the git history of a specified file to extract:
# 1. The date of the very first commit.
# 2. A unique, sorted list of all authors who have contributed to the file.
#
# It uses the '--follow' flag to ensure that the file's history is
# traced even across renames.
#
# It outputs the results in a C++ comment block format and consolidates
# and consolidates author aliases.

# --- Usage ---
# ./analyze_history.sh <path/to/your/file>
#
# Example:
# ./analyze_history.sh src/HourlyModel.cpp

# Check if a file path is provided as an argument.
if [ -z "$1" ]; then
  echo "Usage: $0 <file_path>" >&2
  exit 1
fi

FILE_PATH="$1"

# Verify that the provided path is a file.
if [ ! -f "$FILE_PATH" ]; then
  echo "Error: File not found at '$FILE_PATH'" >&2
  exit 1
fi

# --- Get First Commit Date ---
# `git log --follow` traces the file's history across renames.
# `--format=%ad` specifies the author date.
# `--date=short` formats the date as YYYY-MM-DD.
# `tail -1` gets the last line from the log output, which is the first commit.
FIRST_COMMIT_DATE=$(git log --follow --format=%ad --date=short "$FILE_PATH" | tail -1)

if [ -z "$FIRST_COMMIT_DATE" ]; then
  echo "Error: Could not determine the first commit date for '$FILE_PATH'. Is this file tracked by git?" >&2
  exit 1
fi

# --- Get Author List ---
# `--format='%an'` gets the author's name for each commit.
# `sed` consolidates author aliases.
# `sort | uniq` processes the list to ensure it's sorted and unique.
AUTHORS=$(git log --follow --format='%an' "$FILE_PATH" | sed 's/^muehleisen$/Ralph Muehleisen/' | sort | uniq)

# --- Output in C++ Comment Format ---
echo "// First Commit: $FIRST_COMMIT_DATE"
echo "//"
echo "// Authors:"
# Print each author on a new line for clarity.
while IFS= read -r author; do
  echo "// - $author"
done <<< "$AUTHORS"
