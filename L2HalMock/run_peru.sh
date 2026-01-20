#!/bin/bash

# Export your environment variables


echo "GIT_USERNAME: $GIT_USERNAME"
echo "GIT_PASSWORD: $GIT_PASSWORD"

envsubst < peru.yaml > peru_temp.yaml
echo "Substituted peru.yaml:"
cat peru_temp.yaml
# Move the substituted file to replace the original peru.yaml
chmod 777 peru_temp.yaml
rm -rf peru.yaml
mv peru_temp.yaml peru.yaml
cat peru.yaml

chmod 644 peru.yaml
# Run peru commands
echo "Running peru sync..."
#peru sync --no-cache -v

#peru sync && peru sync --no-cache
#peru sync --no-cache

# Clean up
#rm peru.yaml
