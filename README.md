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
git clone https://github.com/dev-wasm/dev-wasm-dotnet
cd dev-wasm-dotnet
code ./
```

Visual studio should prompt you to see if you want to relaunch the workspace in a container, you do.

# Building and Running

## Simple example
```sh
dotnet build starter
wasmtime --dir . starter/bin/Debug/net7.0/starter.wasm
```

## Web serving
There is a simple example of web serving via WebAssembly + CGI (WAGI) in the www directory. It uses the lighttpd web server and mod_cgi. See the www/lighttpd.conf file for more details.

```sh
dotnet build www
lighttpd -D -f www/lighttpd.conf 
```

Once the server is running, VS Code or Codespaces should prompt you to connect to the open port.
