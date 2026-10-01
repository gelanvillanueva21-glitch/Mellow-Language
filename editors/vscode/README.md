# Mellow Language Support for VS Code

This extension registers `.mll` files as Mellow source and provides syntax
highlighting, bracket matching, comments, and block indentation.

To install it locally from the repository root:

```sh
cd editors/vscode
npx --yes @vscode/vsce package --no-dependencies
code --install-extension mellow-language-0.1.0.vsix
```

Reload the VS Code window after installation. The generated VSIX is local build
output and should not be committed.