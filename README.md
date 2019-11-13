# mongoose-sgx

### Step 1.
- [x] get big upload working without filesystem but with http multipart support
- ~~test using `curl -F data=@file.txt localhost:8000/upload`~~
- simple upload endpoint form working (open localhost:8000 in browser)

### Step 2.
- [x] configure axios for get and post
- run `live-server` in frontend/

### Step 3.
- [ ] setup https://github.com/blueimp/jQuery-File-Upload/wiki/Basic-plugin


### Notes: 
- Able to do multipart requests but not handle content-range


### Issues:
- Content-Disposition: 

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
