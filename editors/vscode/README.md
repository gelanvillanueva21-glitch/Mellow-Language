# Mellow Language Support for VS Code

This extension registers `.mll` files as Mellow source and provides syntax
highlighting, bracket matching, comments, and block indentation.

To install it locally from the repository root:

```sh
cd editors/vscode
npx --yes @vscode/vsce package --no-dependencies
code --install-extension mellow-language-0.1.0.vsix
```

Reload the VS Code window after installation. To enable the `.mll` logo, run
**Preferences: File Icon Theme** from the Command Palette and select **Mellow
File Icons**. Selecting an icon theme is a global VS Code setting; file types
without a Mellow icon may use generic fallback icons.

The generated VSIX is local build output and should not be committed.