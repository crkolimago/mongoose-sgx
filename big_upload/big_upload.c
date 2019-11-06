// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved
//
// This example demonstrates how to handle very large requests without keeping
// them in memory.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mongoose.h"

#define FILE_OK 0
#define FILE_NOT_EXIST 1
#define FILE_TO_LARGE 2
#define FILE_READ_ERROR 3

static const char *s_http_port = "8000";

struct file_writer_data {
  size_t bytes_written;
};

char * c_read_file(const char * f_name, int * err, size_t * f_size) {
    char * buffer;
    size_t length;
    FILE * f = fopen(f_name, "rb");
    size_t read_length;

    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);

        buffer = (char *)malloc(length + 1);

        if (length) {
            read_length = fread(buffer, 1, length, f);

            if (length != read_length) {
                 *err = FILE_READ_ERROR;

                 return NULL;
            }
        }

        fclose(f);

        *err = FILE_OK;
        buffer[length] = '\0';
        *f_size = length;
    }
    else {
        *err = FILE_NOT_EXIST;

        return NULL;
    }

    return buffer;
}

void debug_value(int ev) {
  switch (ev) {
    case MG_EV_HTTP_MULTIPART_REQUEST: {
      printf("%s\n", "MG_EV_HTTP_MULTIPART_REQUEST");
      break;
    }
    case MG_EV_HTTP_PART_BEGIN: {
      printf("%s\n", "MG_EV_HTTP_PART_BEGIN");
      break;
    }
    case MG_EV_HTTP_PART_DATA: {
      printf("%s\n", "MG_EV_HTTP_PART_DATA");
      break;
    }
    case MG_EV_HTTP_PART_END: {
      printf("%s\n", "MG_EV_HTTP_PART_END");
      break;
    }
    case MG_EV_HTTP_MULTIPART_REQUEST_END: {
      printf("%s\n", "MG_EV_HTTP_MULTIPART_REQUEST_END");
      break;
    }
    case MG_EV_HTTP_REQUEST: {
      printf("%s\n", "MG_EV_HTTP_REQUEST");
      break;
    }
    case MG_EV_CLOSE: {
      printf("%s\n", "MG_EV_CLOSE");
      break;
    }
    default: {
      printf("%d (add to logging information)\n", ev);
      break;
    }
  }
}

static void handle_upload(struct mg_connection *nc, int ev, void *p) {
  struct http_message *hm = (struct http_message *) p;

  // logging information
  printf("%p: ", nc);
  debug_value(ev);

  // printf("%.*s\n", (int)hm->method.len, hm->method.p);

  switch (ev) {
    case MG_EV_HTTP_REQUEST: {
      printf("%.*s", (int)hm->message.len, hm->message.p);

      if (!mg_vcmp(&hm->method, "OPTIONS")) {
        mg_send_response_line(nc, 200, NULL);
        mg_printf(nc, "%s",
                  "Access-Control-Allow-Origin: *\r\n"
                  "Access-Control-Allow-Headers: content-disposition,content-type\r\n"
                  "Allow: GET, POST, HEAD, CONNECT, OPTIONS"
                  "\r\n\r\n");
        nc->flags |= MG_F_SEND_AND_CLOSE;
      } else {
        /* Send headers */
        //mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nTransfer-Encoding: chunked\r\n\r\n");

        /* Compute the result and send it back as a JSON object */
        //mg_printf_http_chunk(nc, "{ \"result\": success }");
        //mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */


        // struct mg_str *cd_hdr = mg_get_http_header(hm, "Content-Disposition");
        // printf("%.*s\n", (int)cd_hdr->len, cd_hdr->p);

        // struct mg_str *cl_hdr = mg_get_http_header(hm, "Content-Length");
        // printf("%.*s\n", (int)cl_hdr->len, cl_hdr->p);

        mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\r\n");

        printf(
        "{\"files\":[{"
        "\"name\":\"31tvcol-greatpumpkin-articleLarge.jpg\","
        "\"size\":40496,"
        "\"url\":\"http://example.org/files/picture1.jpg\","
        "\"thumbnailUrl\":\"http://example.org/files/picture1.jpg\","
        "\"deleteUrl\":\"http://example.org/files/picture1.jpg\","
        "\"deleteType\":\"DELETE\""
        "}]}\n"
        );

        mg_printf(nc,
        "{\"files\":[{"
        "\"name\":\"31tvcol-greatpumpkin-articleLarge.jpg\","
        "\"size\":40496,"
        "\"url\":\"http://example.org/files/picture1.jpg\","
        "\"thumbnailUrl\":\"http://example.org/files/picture1.jpg\","
        "\"deleteUrl\":\"http://example.org/files/picture1.jpg\","
        "\"deleteType\":\"DELETE\""
        "}]}");

        nc->flags |= MG_F_SEND_AND_CLOSE;
        
      }

      break;
    }
  }
}

// look at mg_http_parse_range_header

/*
static void handle_upload(struct mg_connection *nc, int ev, void *p) {
  struct file_writer_data *data = (struct file_writer_data *) nc->user_data;
  struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) p;

  // im guessing this is super unsafe but for debugging purposes
  if ((void *)mp) {
    if (mp->status < 0) {
      printf("REQUEST:----------\n%s\n----------\n",mp->file_name);
    }
  }

  // logging information
  printf("%p: ", nc);
  debug_value(ev);

  switch (ev) {
    case MG_EV_HTTP_PART_BEGIN: {
      if (data == NULL) {
        data = calloc(1, sizeof(struct file_writer_data));
        data->bytes_written = 0;

        nc->user_data = (void *) data;
      }
      break;
    }
    case MG_EV_HTTP_PART_DATA: {
      // this is where the actual data is processed

      printf("data_length: %ld\n", mp->data.len);
      data->bytes_written += mp->data.len;

      break;
    }
    case MG_EV_HTTP_PART_END: {
      //mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");

      //result = strtod(n1, NULL) + strtod(n2, NULL);
      //mg_printf_http_chunk(nc, "{ \"result\": %lf }", result);
      //mg_send_http_chunk(nc, "", 0);

      
      mg_printf(nc,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n"
                "Received %ld bytes of POST data\r\n\r\n",
                data->bytes_written);
      
      nc->flags |= MG_F_SEND_AND_CLOSE;
      free(data);
      nc->user_data = NULL;
      break;
    }
    case MG_EV_HTTP_REQUEST: {
      struct http_message *hm = (struct http_message *) p;

      // We have received an HTTP request. Parsed request is contained in `hm`.
      // Send HTTP reply to the client which shows full original request.
      mg_send_head(nc, 200, hm->message.len,
      "Content-Type: text/plain"
      );
      printf("%.*s", (int)hm->message.len, hm->message.p);
      mg_printf(nc, "%.*s", (int)hm->message.len, hm->message.p);
    }
  }
}
*/

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  /*
  if (ev == MG_EV_HTTP_REQUEST) {
    struct http_message *hm = (struct http_message *) ev_data;

    // We have received an HTTP request. Parsed request is contained in `hm`.
    // Send HTTP reply to the client which shows full original request.
    mg_send_head(nc, 200, hm->message.len,
    "Content-Type: text/plain\r\n"
    "Access-Control-Allow-Origin: http://127.0.0.1:8080"
    );
    printf("%.*s", (int)hm->message.len, hm->message.p);
    mg_printf(nc, "%.*s", (int)hm->message.len, hm->message.p);
  }
  */
 

  int err;
  size_t f_size;
  char * f_data;

  // GET REQUEST - SERVE INDEX
  if (ev == MG_EV_HTTP_REQUEST) {
    struct http_message *hm = (struct http_message *) ev_data;

    // logging information
    printf("%p: %.*s %.*s\r\n", nc, (int) hm->method.len, hm->method.p,
             (int) hm->uri.len, hm->uri.p);

    mg_send_response_line(nc, 200,
                            "Content-Type: text/html\r\n"
                            "Connection: close");

    f_data = c_read_file("index.html", &err, &f_size);

    if (err) {
        // process error
    }

    mg_printf(nc, "%s", f_data);

    free(f_data);

    nc->flags |= MG_F_SEND_AND_CLOSE;

  }

    /* Reversing mg_serve_http:
    * mg_http_serve_file2
    * mg_http_serve_file [complicated]
    * mg_fopen (fopen(path, "rb") but with extra stuff for windows directory)
    * mg_http_transfer_file_data
    * Note: this has proved to be a very challenging task without filesystem support so going
    * to use a loop just to get serving files working
    * */
}

int main(void) {
  struct mg_mgr mgr;
  struct mg_connection *c;

  mg_mgr_init(&mgr, NULL);
  c = mg_bind(&mgr, s_http_port, ev_handler);
  if (c == NULL) {
    fprintf(stderr, "Cannot start server on port %s\n", s_http_port);
    exit(EXIT_FAILURE);
  }

  mg_register_http_endpoint(c, "/upload", handle_upload MG_UD_ARG(NULL));

  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(c);

  printf("Starting web server on port %s\n", s_http_port);
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}
