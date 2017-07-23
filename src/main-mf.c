#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

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
static gchar *subject = "You need this JAB!";
static gchar *message = "Don't be a fool, take your meds!";
static gchar *resource = "NULL";

static GOptionEntry entries[] =
{
    { "auths", 'a', 0, G_OPTION_ARG_STRING, &authsfile,
      "Authentications file list (e.g. auth.list)", NULL },
    { "threads", 't', 0, G_OPTION_ARG_INT, &numthreads,
      "Number of threads to use [default=1]", NULL },
    { "recipient", 'R', 0, G_OPTION_ARG_STRING, &recipient,
      "Recipient to send the message to (e.g. user@server.org)", NULL },
    { "subject", 's', 0, G_OPTION_ARG_STRING, &subject,
      "Subject line to send [default=You need this JAB!]", NULL },
    { "message", 'm', 0, G_OPTION_ARG_STRING, &message,
      "Message to send to recipient [default=Don't be a fool, take your meds!]", NULL },
    { "resource", 'r', 0, G_OPTION_ARG_STRING, &resource,
      "Resource to connect with [default=NULL]", NULL },
    { NULL }
};

void thread_messager(void *line)
{
    LmConnection *lconnection;

    gchar *pubserv = strtok(line, "|");
    gchar *conserv = strtok(NULL, "|");
    gchar *username = strtok(NULL, "|");
    gchar *password = strtok(NULL, "|");
    gchar *conport = strtok(conserv, ":");
    conport = strtok(NULL, ":");

    if(!pubserv || !conserv || !conport)
        return;

    printf("pubserv: %s, conserv: %s, username: %s, password: %s, conport: %s\n",
        pubserv, conserv, username, password, conport);

    gint jconport = (gint)atoi(conport);

    //main_context = g_main_context_new();

    lconnection = xmpp_connect(pubserv, conserv, jconport,
        username, password, resource, FALSE);

    xmpp_sendmsg(recipient, subject, message, lconnection);

    xmpp_disconnect(lconnection);

    return;
}

int main(int argc, char **argv)
{
    FILE *fp;
    size_t len = 0;
    ssize_t read;
    char *line = NULL;

    GOptionContext *context;

    context = g_option_context_new("- Jabinator - Message flooder");
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
        gint i;
        pthread_t thread[numthreads];
        // TODO: Add threading

        line[strcspn(line, "\n")] = 0;

        for(i = 0; i < numthreads; i++)
        {
            pthread_create(&thread[i], NULL, (void*)&thread_messager, (void*)line);
            pthread_detach(thread[i]);
            g_usleep(500000);
        }
    }

    while(1);

    return EXIT_SUCCESS;
}
