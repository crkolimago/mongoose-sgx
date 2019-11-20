// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved
//
// This example demonstrates how to handle very large requests without keeping
// them in memory.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mongoose.h"

#define MAX_FILE_SIZE 1000000


#ifndef MG_MALLOC
#define MG_MALLOC malloc
#endif

#ifndef MG_FREE
#define MG_FREE free
#endif

static const char *s_http_port = "8000";

struct file_writer_data {
  size_t bytes_written;
  int64_t range_start;
  int64_t range_end;
  int64_t range_total;
  char *filename;
};

static void debug_value(int ev) {
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

static int parse_content_range_header(const struct mg_str *header, int64_t *a,
                                      int64_t *b, int64_t *c) {
  int result;
  char *p = (char *) MG_MALLOC(header->len + 1);
  if (p == NULL) return 0;
  memcpy(p, header->p, header->len);
  p[header->len] = '\0';
  result = sscanf(p, "bytes %" INT64_FMT "-%" INT64_FMT "/%" INT64_FMT, a, b, c);
  MG_FREE(p);
  return result;
}

static void handle_upload(struct mg_connection *nc, int ev, void *p) {
  struct http_message *hm = (struct http_message *) p;
  struct file_writer_data *data = (struct file_writer_data *) nc->user_data;
  struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) p;

  switch (ev) {
    case MG_EV_HTTP_REQUEST: {
      printf("%.*s", (int)hm->message.len, hm->message.p);

      // jQuery File Uploader first sends a request for options - send standard HTTP methods
      if (!mg_vcmp(&hm->method, "OPTIONS")) {
        mg_send_response_line(nc, 200, NULL);
        mg_printf(nc, "%s",
                  "Access-Control-Allow-Origin: *\r\n"
                  "Access-Control-Allow-Headers: content-disposition, content-type, content-range\r\n"
                  "Connection: close\r\n"
                  "Allow: GET, POST, HEAD, CONNECT, OPTIONS"
                  "\r\n\r\n");
        nc->flags |= MG_F_SEND_AND_CLOSE;
      }
      break;
    }
    case MG_EV_HTTP_MULTIPART_REQUEST: {
      int n, filename_len;
      char range[70], filename_buf[100];
      char *filename = filename_buf;
      int64_t r1 = 0, r2 = 0, r3 = 0;
      struct mg_str *range_hdr = mg_get_http_header(hm, "Content-Range");
      struct mg_str *file_hdr = mg_get_http_header(hm, "Content-Disposition");

      range[0] = '\0';
      if (range_hdr != NULL &&
          (n = parse_content_range_header(range_hdr, &r1, &r2, &r3)) > 0 && r1 >= 0 &&
          r2 >= 0 && r3 >= 0) {
        snprintf(range, sizeof(range), "Content-Range: bytes %" INT64_FMT
          "-%" INT64_FMT "/%" INT64_FMT "\r\n", r1, r2, r3);
      }

      filename_len =
        mg_http_parse_header2(file_hdr, "filename", &filename, sizeof(filename_buf));
      if (filename != filename_buf) {
          MG_FREE(filename);
          filename_buf[0] = '\0';
      } else if (filename_len == 0) {
        // something like this
        nc->flags = MG_F_CLOSE_IMMEDIATELY;
      }

      if (data == NULL) {
        data = calloc(1, sizeof(struct file_writer_data));
        data->filename = filename_buf;
        data->range_start = r1;
        data->range_end = r2;
        data->range_total = r3;

        nc->user_data = (void *) data;
      }

      break;
    }
    case MG_EV_HTTP_PART_BEGIN: {
      data->bytes_written = 0;
      break;
    }
    case MG_EV_HTTP_PART_DATA: {
      /*if (fwrite(mp->data.p, 1, mp->data.len, data->fp) != mp->data.len) {
        mg_printf(nc, "%s",
                  "HTTP/1.1 500 Failed to write to a file\r\n"
                  "Content-Length: 0\r\n\r\n");
        nc->flags |= MG_F_SEND_AND_CLOSE;
        return;
      }*/

      data->bytes_written += mp->data.len;

      break;
    }
    //case MG_EV_HTTP_MULTIPART_REQUEST_END: {
    case MG_EV_HTTP_PART_END: {
      mg_printf(nc,
      "HTTP/1.1 200 OK\r\n"
      "Access-Control-Allow-Origin: *\r\n"
      "Content-Type: application/json\r\n"
      "Connection: close\r\n\r\n"
      "{\"files\":[{"
      "\"name\":\"mb_file.txt\","
      "\"size\":%ld,"
      "\"url\":\"http://example.org/files/a\","
      "\"thumbnailUrl\":\"http://example.org/files/a\","
      "\"deleteUrl\":\"http://example.org/files/a\","
      "\"deleteType\":\"DELETE\""
      "}]}\r\n\r\n",
      (long)data->bytes_written);

      nc->flags |= MG_F_SEND_AND_CLOSE;
      free(data);
      nc->user_data = NULL;
    }
  }
}

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
