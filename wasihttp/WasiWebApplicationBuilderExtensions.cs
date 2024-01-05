using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting.Server;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Http.Features;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Wasi.Http;

namespace Microsoft.AspNetCore.Builder;

public static class WasiWebApplicationBuilderExtensions
{
    public static WebApplicationBuilder UseWasiServer(this WebApplicationBuilder builder)
    {
        builder.Services.AddSingleton<IServer, WasiServer>();
        // builder.Services.AddScoped<IdentAccessor>();
        builder.Services.AddHttpContextAccessor();
        // builder.Logging.AddProvider(new WasiLoggingProvider());
        return builder;
    }
}