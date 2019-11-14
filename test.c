#include <string.h>
#include <stdio.h>

int main() {
    char str[27] = "bytes 200000-215506/215506";
    char var_name[6] = "bytes";

    int str_length = strlen(str);
    int var_name_length = strlen(var_name);

    printf("%.*s %d\n", str_length, str, str_length);
    printf("%.*s %d\n", var_name_length, var_name, var_name_length);

    char * s;

    s = str;
    s = s + (var_name_length+1);
    printf("%s\n", s);
    
    char *slash_token, *dash_token;
    const char sep1[2] = "/", sep2[2] = "-";
    
    slash_token = strtok(s, sep1);

    while (slash_token != NULL) {
        printf("/: %s\n", slash_token);
        dash_token = strtok(slash_token, sep2);
        while (dash_token != NULL) {
            printf("-: %s\n", dash_token);
            dash_token = strtok(NULL, sep1);
        }
        slash_token = strtok(NULL, sep1);
    }

    /*
    const char[2] ch1 = '/', ch2 = '-';
    int n = strlen(var_name);
    const char *p, *token_slash, *token_dash, *end = hdr ? hdr->p + hdr->len : NULL, *s = NULL;
    s = hdr->p;

    if (s != NULL && &s[n + 1] < end) {
    s += n + 1;
    p = s;
    // split on '/'
    token_slash = strtok(p, ch1);
    while(token_slash != NULL) {
        printf("%s\n", token_slash);
        token_dash = strtok(p, ch1);
        token_slash = strtok(NULL, ch1);
    }
    }
    */
}