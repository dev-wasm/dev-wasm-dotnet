# Experimental HTTP Server Example
*NB*: this example uses an experimental `wasi-http` that is part of the WASI specification
but is still going through iteration. Expect breaking changes in future releases.

## Building
```sh
dotnet build
```

## Running
```sh
# Run the server
make run

# Connect to the server
curl localhost:8080
```