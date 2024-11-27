#!/bin/bash

cd .git/hooks
if [ ! -f pre-commit ]; then
    cat << 'EOF' > pre-commit
#!/bin/bash

GIT_ROOT="$(git rev-parse --show-toplevel)"

(cd ${GIT_ROOT}/pcb/emonpi3/ && ./generate.py)

git diff --staged --name-only | xargs git add

EOF
chmod +x pre-commit
fi
