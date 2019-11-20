# mongoose-sgx

### Run Client
`frontend$ live-server`

### Run Server
`big_upload$ ./big_upload`

### TODO: 
- Handle filename processing
- Handle cryptographic no-op

### Compile flags

CS_ENABLE_STDIO=0
MG_ENABLE_BROADCAST=0
MG_ENAGLE_DIRECTORY_LISTING=0
MG_ENABLE_FILESYSTEM=0
MG_ENABLE_HTTP_CGI=0
MG_ENABLE_GETADDRINFO=0
MG_ENABLE_SYNC_RESOLVER=0
MG_ENABLE_ASYNC_RESOLVER=0
MG_ENABLE_SNTP=0
MG_ENABLE_MQTT=0
MG_ENABLE_HTTP_STREAMING_MULTIPART=1
