# Devcontainer WASM-dotnet
Simple devcontainer for dotnet development

# Usage

## Github Codespaces
Just click the button:

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://github.com/codespaces/new?hide_repo_select=true&ref=main&repo=575630606)

## Visual Studio Code
Note this assumes that you have the VS code support for remote containers and `docker` installed 
on your machine.

```sh
git clone https://github.com/brendandburns/dev-wasm-dotnet
cd dev-wasm-dotnet
code ./
```

Visual studio should prompt you to see if you want to relaunch the workspace in a container, you do.

# Building and Running

```sh
dotnet build starter
wasmtime starter/bin/Debug/net7.0/starter.wasm
```
