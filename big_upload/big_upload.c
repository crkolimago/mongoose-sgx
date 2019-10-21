// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved
//
// This example demonstrates how to handle very large requests without keeping
// them in memory.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mongoose.h"

static const char *s_http_port = "8000";

struct file_writer_data {
  size_t bytes_written;
};

static void handle_upload(struct mg_connection *nc, int ev, void *p) {
  struct file_writer_data *data = (struct file_writer_data *) nc->user_data;
  struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) p;

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

      data->bytes_written += mp->data.len;

      break;
    }
    case MG_EV_HTTP_PART_END: {
      mg_printf(nc,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n"
                "Received %ld bytes of POST data\n\n",
                data->bytes_written);
      nc->flags |= MG_F_SEND_AND_CLOSE;
      free(data);
      nc->user_data = NULL;
      break;
    }
  }
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_REQUEST) {
    struct http_message *hm = (struct http_message *) ev_data;

    // logging information
    printf("%p: %.*s %.*s\r\n", nc, (int) hm->method.len, hm->method.p,
             (int) hm->uri.len, hm->uri.p);

    mg_send_response_line(nc, 200,
                            "Content-Type: text/html\r\n"
                            "Connection: close");
    mg_printf(nc,
              "\r\n<form method='POST' action='/upload' enctype='multipart/form-data'>"
              "<input type='file' name='file'>"
              "<input type='submit' value='Upload'>"
              "</form>\r\n");

    nc->flags |= MG_F_SEND_AND_CLOSE;

    /* Additional Error Checking not handled !!!!
    
    // mg_serve_http(nc, ev_data, s_http_server_opts);

    void mg_serve_http(struct mg_connection *nc, struct http_message *hm,
                      struct mg_serve_http_opts opts) {
      char *path = NULL;
      struct mg_str *hdr, path_info;
      uint32_t remote_ip = ntohl(*(uint32_t *) &nc->sa.sin.sin_addr);

      if (mg_check_ip_acl(opts.ip_acl, remote_ip) != 1) {
        // Not allowed to connect
        mg_http_send_error(nc, 403, NULL);
        nc->flags |= MG_F_SEND_AND_CLOSE;
        return;
      }

    #if MG_ENABLE_HTTP_URL_REWRITES
      if (mg_http_handle_forwarding(nc, hm, &opts)) {
        return;
      }

      if (mg_http_send_port_based_redirect(nc, hm, &opts)) {
        return;
      }
    #endif

      if (opts.document_root == NULL) {
        opts.document_root = ".";
      }
      if (opts.per_directory_auth_file == NULL) {
        opts.per_directory_auth_file = ".htpasswd";
      }
      if (opts.enable_directory_listing == NULL) {
        opts.enable_directory_listing = "yes";
      }
      if (opts.cgi_file_pattern == NULL) {
        opts.cgi_file_pattern = "**.cgi$|**.php$";
      }
      if (opts.ssi_pattern == NULL) {
        opts.ssi_pattern = "**.shtml$|**.shtm$";
      }
      if (opts.index_files == NULL) {
        opts.index_files = "index.html,index.htm,index.shtml,index.cgi,index.php";
      }
      // Normalize path - resolve "." and ".." (in-place). 
      if (!mg_normalize_uri_path(&hm->uri, &hm->uri)) {
        mg_http_send_error(nc, 400, NULL);
        return;
      }
      if (mg_uri_to_local_path(hm, &opts, &path, &path_info) == 0) {
        mg_http_send_error(nc, 404, NULL);
        return;
      }
      mg_send_http_file(nc, path, &path_info, hm, &opts);

      MG_FREE(path);
      path = NULL;

      // Close connection for non-keep-alive requests 
      if (mg_vcmp(&hm->proto, "HTTP/1.1") != 0 ||
          ((hdr = mg_get_http_header(hm, "Connection")) != NULL &&
          mg_vcmp(hdr, "keep-alive") != 0)) {
    #if 0
        nc->flags |= MG_F_SEND_AND_CLOSE;
    #endif
      }
    }
    */
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
