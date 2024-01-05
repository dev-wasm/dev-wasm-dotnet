// This is adapted from https://github.com/dotnet/runtime/blob/main/src/libraries/System.Net.Http/src/System/Net/Http/BrowserHttpHandler/BrowserHttpHandler.cs
// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Net;
using System.Net.Http.Headers;
using System.Net.Security;

namespace Wasi.Http
{
    public sealed class WasiHttpHandler : HttpMessageHandler
    {
        private static readonly HttpRequestOptionsKey<bool> EnableStreamingResponse = new HttpRequestOptionsKey<bool>("WebAssemblyEnableStreamingResponse");
        private static readonly HttpRequestOptionsKey<IDictionary<string, object>> FetchOptions = new HttpRequestOptionsKey<IDictionary<string, object>>("WebAssemblyFetchOptions");
        private bool _allowAutoRedirect = false;
        // flag to determine if the _allowAutoRedirect was explicitly set or not.
        private bool _isAllowAutoRedirectTouched;

        /// <summary>
        /// Gets whether the current environment supports streaming responses
        /// </summary>
        private static bool StreamingSupported { get; } = GetIsStreamingSupported();
        private static bool GetIsStreamingSupported() { return false; }

        public bool UseCookies
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        public CookieContainer CookieContainer
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        public DecompressionMethods AutomaticDecompression
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        public bool UseProxy
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        public IWebProxy? Proxy
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        public ICredentials? DefaultProxyCredentials
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        public bool PreAuthenticate
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        public ICredentials? Credentials
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        // TODO: This doesn't currently do anything...
        public bool AllowAutoRedirect
        {
            get => _allowAutoRedirect;
            set
            {
                _allowAutoRedirect = value;
                _isAllowAutoRedirectTouched = true;
            }
        }

        public int MaxAutomaticRedirections
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        public int MaxConnectionsPerServer
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        public int MaxResponseHeadersLength
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        public SslClientAuthenticationOptions SslOptions
        {
            get => throw new PlatformNotSupportedException();
            set => throw new PlatformNotSupportedException();
        }

        public bool SupportsAutomaticDecompression => false;
        public bool SupportsProxy => false;
        public bool SupportsRedirectConfiguration => true;

        private Dictionary<string, object?>? _properties;
        public IDictionary<string, object?> Properties => _properties ??= new Dictionary<string, object?>();


        static WasiHttpExperimental.Scheme GetScheme(HttpRequestMessage request) {
            switch (request.RequestUri!.Scheme) {
                case "http": return WasiHttpExperimental.Scheme.HTTP;
                case "https": return WasiHttpExperimental.Scheme.HTTPS;
            }
            throw new NotSupportedException($"Unknown scheme: {request.RequestUri.Scheme}");
        }

        static WasiHttpExperimental.Method GetMethod(HttpRequestMessage request) {
            switch (request.Method.Method.ToUpper())
            {
            case "GET":
                return WasiHttpExperimental.Method.GET;
            case "POST":
                return WasiHttpExperimental.Method.POST;
            case "PUT":
                return WasiHttpExperimental.Method.PUT;
            case "DELETE":
                return WasiHttpExperimental.Method.DELETE;
            case "HEAD":
                return WasiHttpExperimental.Method.HEAD;
            case "OPTIONS":
                return WasiHttpExperimental.Method.OPTIONS;
            case "TRACE":
                return WasiHttpExperimental.Method.TRACE;
            case "CONNECT":
                return WasiHttpExperimental.Method.CONNECT;
            default:
                throw new ArgumentException("Invalid HTTP method.");
            }
        }

        protected override async Task<HttpResponseMessage> SendAsync(HttpRequestMessage request, CancellationToken cancellationToken)
        {
            int statusCode;
            int handle;
            string body = "";
            string headers = "";
            int headerCount = request.Headers.Count() + request.Content?.Headers.Count() ?? 0;

            if (request.Content != null)
            {
                body = await request.Content.ReadAsStringAsync();
            }

            if (request.RequestUri == null || request.Method == null)
            {
                throw new InvalidOperationException();
            }

            List<string> headerList = new List<string>(headerCount);

            foreach (KeyValuePair<string, IEnumerable<string>> header in request.Headers)
            {
                foreach (string value in header.Value)
                {
                    headerList.Add(header.Key + ": " + value);
                }
            }

            if (request.Content != null)
            {
                foreach (KeyValuePair<string, IEnumerable<string>> header in request.Content.Headers)
                {
                    foreach (string value in header.Value)
                    {
                        headerList.Add(header.Key + ": " + value);
                    }
                }
            }

            headers = String.Join("\n", headerList);

            unsafe
            {
                WasiHttpExperimental.Method method = GetMethod(request);
                WasiHttpExperimental.Scheme scheme = GetScheme(request);

                int err = WasiHttpExperimental.Req(method, scheme, request.RequestUri.Authority, request.RequestUri.LocalPath + request.RequestUri.Query, headers, body, &statusCode, &handle);
                if (err != 0)
                {
                    throw new HttpRequestException(err.ToString());
                }
                headers = WasiHttpExperimental.ReadAllHeaders(handle);
                body = WasiHttpExperimental.ReadBody(handle);
                WasiHttpExperimental.Close(handle);
            }

            var response = new HttpResponseMessage((HttpStatusCode)statusCode);
            var contentType = "";
            var parts = headers.Split("\n");
            foreach (var part in parts)
            {
                var pieces = part.Split(":", 2);
                if (pieces.Length == 2)
                {
                    if (pieces[0].Equals("content-type", StringComparison.InvariantCultureIgnoreCase))
                    {
                        contentType = pieces[1];
                    }
                    else
                    {
                        response.Headers.TryAddWithoutValidation(pieces[0], pieces[1]);
                    }
                }
            }
            MediaTypeHeaderValue? contentHeader = null;
            if (contentType.Length > 0)
            {
                try
                {
                    contentHeader = MediaTypeHeaderValue.Parse(contentType);
                }
                catch (System.FormatException ex)
                {
                    Console.WriteLine("Unparseable content: " + contentType + ex);
                }
            }
            if (contentHeader != null)
            {
                response.Content = new StringContent(body, contentHeader);
            }
            else
            {
                response.Content = new StringContent(body);
            }
            return response;
        }
    }
}