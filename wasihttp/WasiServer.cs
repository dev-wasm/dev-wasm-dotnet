namespace Wasi.Http;

using Microsoft.AspNetCore.Hosting.Server;
using Microsoft.AspNetCore.Hosting.Server.Features;
using Microsoft.AspNetCore.Http.Features;
using Microsoft.Extensions.Hosting;

public class WasiServer : IServer {
    private readonly IHostApplicationLifetime _lifetime;

    public IFeatureCollection Features { get; } = new FeatureCollection();

    protected void Run<TContext>(IHttpApplication<TContext> application, int port) where TContext : notnull {

    }

    public WasiServer(IHostApplicationLifetime lifetime)
    {
        _lifetime = lifetime;
        Features[typeof(IServerAddressesFeature)] = this;
    }

    public Task StartAsync<TContext>(IHttpApplication<TContext> application, CancellationToken cancellationToken) where TContext : notnull
    {
        var addresses = ((IServerAddressesFeature)this).Addresses;
        if (addresses.Count == 0)
        {
            addresses.Add("http://localhost:8080");
        }
        var port = Uri.TryCreate(addresses.First().Replace("*", "host"), default, out var uri)
            ? uri.Port : 8080;

        _lifetime.ApplicationStarted.Register(() => Run(application, port));

        return Task.CompletedTask;
    }

    public Task StopAsync(CancellationToken cancellationToken)
        => Task.CompletedTask;

    public void Dispose()
    {
    }
}