#!/bin/bash

# Define variables
REPO_PATH="$HOME/parasoft/workspace_BIU_TEST/sensor_c"  # Set this to your project path
WORKFLOW_ID="153015810"  # GitHub Actions workflow ID
COMMIT_MESSAGE="Automated: Pushing C file changes and triggering GitHub Actions"

# Move to the repository
cd "$REPO_PATH" || exit 1  # Exit if the directory is missing

# Check for .c file changes
if git diff --name-only | grep -E "\.c$"; then
    echo "Detected changes in .c files, committing and pushing..."
    
    # Add changed .c files
    git add *.c
    
    # Commit changes
    git commit -m "$COMMIT_MESSAGE"
    
    # Push to GitHub
    git push origin main
    
    # Trigger GitHub Actions workflow
    echo "Triggering GitHub Actions workflow..."
    gh workflow run "$WORKFLOW_ID"

    echo "✅ Changes pushed and workflow triggered successfully!"
else
    echo "No .c file changes detected. Exiting."
fi
