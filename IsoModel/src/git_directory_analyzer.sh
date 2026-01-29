#!/bin/bash

# --- Git History Analyzer ---
#
# This script analyzes the git history of all .cpp and .hpp files in the
# current directory and subdirectories to extract:
# 1. The date of the very first commit.
# 2. A unique, sorted list of all authors who have contributed to the file.
#
# It uses the '--follow' flag to ensure that the file's history is
# traced even across renames.
#
# It outputs the results in a structured format with C++ style comments
# and consolidates author aliases.

# --- Usage ---
# ./analyze_history.sh
#
# Example:
# ./analyze_history.sh > all_headers.txt

# Find all .cpp and .hpp files and loop through them
find . -name "*.cpp" -o -name "*.hpp" | while read -r FILE_PATH; do
  # --- Start of block for this file ---
  echo "--- FILE: $FILE_PATH ---"

  # --- Get First Commit Date ---
  # `git log --follow` traces the file's history across renames.
  # `--format=%ad` specifies the author date.
  # `--date=short` formats the date as YYYY-MM-DD.
  # `tail -1` gets the last line from the log output, which is the first commit.
  FIRST_COMMIT_DATE=$(git log --follow --format=%ad --date=short "$FILE_PATH" | tail -1)

  if [ -z "$FIRST_COMMIT_DATE" ]; then
    echo "Error: Could not determine the first commit date for '$FILE_PATH'." >&2
    continue
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

  # Add a blank line for readability between files
  echo ""
done
