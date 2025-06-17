#!/bin/bash

TOKEN="<access_token>"
PROJECT_ID="14252974"
PACKAGE_NAME="valentina"
PACKAGE_VERSION="0.7.53"
DIRECTORY_PATH="./0.7.53"

for file in "$DIRECTORY_PATH"/*; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        echo "Uploading: $filename"
        curl -o /tmp/u.tmp -# \
             --location --silent --show-error \
             --header "PRIVATE-TOKEN: $TOKEN" \
             --upload-file "$file" \
             "https://gitlab.com/api/v4/projects/$PROJECT_ID/packages/generic/$PACKAGE_NAME/$PACKAGE_VERSION/$filename" \
             && cat /tmp/u.tmp && rm /tmp/u.tmp 
        echo "Uploaded: $filename"
    fi
done
