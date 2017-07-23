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

static gchar *resource = NULL;
static gchar *recipient = NULL;
static gchar *message = "test asynchronous message";

static GOptionEntry entries[] =
{
    { "resource", 'r', 0, G_OPTION_ARG_STRING, &resource,
      "Resource connect with [default=lm-send-async]", NULL },
    { "recipient", 'R', 0, G_OPTION_ARG_STRING, &recipient,
      "Recipient to send the message to (e.g. user@server.org)", NULL },
    { "message", 'm', 0, G_OPTION_ARG_STRING, &message,
      "Message to send to recipient [default=test message]", NULL },
    { NULL }
};

int main(int argc, char **argv)
{
    FILE *fp;
    size_t len = 0;
    ssize_t read;
    char *line = NULL;

    GOptionContext *context;

    context = g_option_context_new ("- test send message asynchronously");
    g_option_context_add_main_entries (context, entries, NULL);
    g_option_context_parse (context, &argc, &argv, NULL);
    g_option_context_free (context);

    if(recipient && strchr(recipient, '@') == NULL)
    {
        g_printerr ("LmSendAsync: Username must have an '@' included\n");
        return EXIT_FAILURE;
    }

    fp = fopen("auth.list", "r");
    if(fp == NULL)
    {
        g_print("auth.list doesn't exist!\n");
        return -1;
    }

    while((read = getline(&line, &len, fp)) != -1)
    {
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

        gchar *jid = "jid@domain.tld";

        xmpp_addjid(jid, lconnection);
        xmpp_deljid(jid, lconnection);
        //xmpp_sendmsg(jid, "LOL", "Hello", lconnection);

        break;
    }

    return 0;
}
