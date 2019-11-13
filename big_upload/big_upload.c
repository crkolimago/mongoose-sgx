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
  struct file_writer_data *data = (struct file_writer_data *) nc->user_data;
  struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) p;

  // logging information
  printf("%p: ", nc);
  debug_value(ev);

  switch (ev) {
    case MG_EV_HTTP_REQUEST: {
      printf("%.*s", (int)hm->message.len, hm->message.p);

      // jQuery File Uploader first sends a request for options - send standard HTTP methods
      if (!mg_vcmp(&hm->method, "OPTIONS")) {
        mg_send_response_line(nc, 200, NULL);
        mg_printf(nc, "%s",
                  "Access-Control-Allow-Origin: *\r\n"
                  "Access-Control-Allow-Headers: content-disposition,content-type, content-range\r\n"
                  "Connection: close\r\n"
                  "Allow: GET, POST, HEAD, CONNECT, OPTIONS"
                  "\r\n\r\n");
        nc->flags |= MG_F_SEND_AND_CLOSE;
      }
      break;
    }
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
      mg_printf(nc,
      "HTTP/1.1 200 OK\r\n"
      "Access-Control-Allow-Origin: *\r\n"
      "Content-Type: application/json\r\n"
      "Connection: close\r\n\r\n"
      "{\"files\":[{"
      "\"name\":\"31tvcol-greatpumpkin-articleLarge.jpg\","
      "\"size\":40496,"
      "\"url\":\"http://example.org/files/picture1.jpg\","
      "\"thumbnailUrl\":\"http://example.org/files/picture1.jpg\","
      "\"deleteUrl\":\"http://example.org/files/picture1.jpg\","
      "\"deleteType\":\"DELETE\""
      "}]}\r\n\r\n");
      
      nc->flags |= MG_F_SEND_AND_CLOSE;
      free(data);
      nc->user_data = NULL;
      break;
    }
  }
}

// look at mg_http_parse_range_header

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_REQUEST) {
    struct http_message *hm = (struct http_message *) ev_data;

    // We have received an HTTP request. Parsed request is contained in `hm`.
    // Send HTTP reply to the client which shows full original request.
    mg_send_head(nc, 200, hm->message.len,
    "Content-Type: text/plain\r\n"
    "Access-Control-Allow-Origin: *"
    );
    printf("%.*s", (int)hm->message.len, hm->message.p);
    mg_printf(nc, "%.*s", (int)hm->message.len, hm->message.p);
  }
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
