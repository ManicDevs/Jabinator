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
#include <hdr/xmpp_iqs.h>

GMainContext *main_context;

static gint connect_port = 5222;
static gchar *public_server = NULL;
static gchar *connect_server = NULL;
static gchar *username = NULL;
static gchar *password = NULL;

static LmSSLResponse ssl_func(LmSSL *ssl, LmSSLStatus status, gpointer user_data)
{
    return LM_SSL_RESPONSE_CONTINUE;
}

static void rand_str(char *dest, size_t length)
{
    const char charset[] =  "0123456789"
                            "abcdefghijklmnopqrstuvwxyz";

    while(length-- != 0)
    {
        size_t index = (double)rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }

    *dest = '\0';
}

int jab_register(gchar *public_s, gchar *connect_s, gint connect_p, int ssl)
{
    char randuser[8] = "";
    char randpass[8] = "";
    gchar *jid = NULL;

    GError *error = NULL;

    gboolean        use_ssl = FALSE;
    LmConnection    *connection;
    LmMessage       *m, *reply;
    LmMessageNode   *query, *node;

    public_server = public_s;
    connect_server = connect_s;
    connect_port = connect_p;
    use_ssl = ssl;

    rand_str(randuser, 7);
    username = randuser;
    rand_str(randpass, 7);
    password = randpass;

    g_print("%s|%s:%d|%s|%s\n", public_server, connect_server, connect_port, username, password);

    connection = lm_connection_new(connect_server);
    jid = g_strdup_printf("%s@%s", username, public_server);
    lm_connection_set_jid(connection, jid);
    g_free(jid);

    if(use_ssl)
    {
        LmSSL *ssl;

        if(!lm_ssl_is_supported())
        {
            g_print("This loudmouth installation doesn't support SSL\n");
            return -1;
        }

        g_print("Setting SSL\n");
        ssl = lm_ssl_new(NULL, ssl_func, NULL, NULL);
        lm_connection_set_ssl(connection, ssl);
        lm_ssl_unref(ssl);
    }

    lm_connection_set_port(connection, connect_port);

    if(!lm_connection_open_and_block(connection, NULL))
    {
        g_print("Failed to open\n");
        return -1;
    }

    m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
    query = lm_message_node_add_child(m->node, "query", NULL);
    lm_message_node_set_attributes(query, "xmlns", "jabber:iq:register", NULL);
    lm_message_node_add_child(query, "username", username);
    lm_message_node_add_child(query, "password", password);

    reply = lm_connection_send_with_reply_and_block(connection, m, &error);
    if(!reply)
        g_print("Failed to send registration request\n");

    FILE *fp;

    switch(lm_message_get_sub_type(reply))
    {
        case LM_MESSAGE_SUB_TYPE_RESULT:
            fp = fopen("auth.list", "a");
            if(fp == NULL)
            {
                g_print("Failed to open auth.list\n");
                return -1;
            }
            fprintf(fp, "%s|%s:%d|%s|%s\n", public_server, connect_server, connect_port, username, password);
            fclose(fp);
            g_print("Succeeded in registering account '%s@%s'\n", username, public_server);
        break;

        case LM_MESSAGE_SUB_TYPE_ERROR:
        default:
            g_print("Failed to register account '%s@%s' due to: ", username, public_server);

            node = lm_message_node_find_child(reply->node, "error");
            if(node)
                g_print("%s\n", lm_message_node_get_value(node));
            else
                g_print("Unknown error\n");
        break;
    }

    lm_connection_close(connection, NULL);

    return 0;
}

int main(int argc, char *argv[])
{
    FILE *fp;
    size_t len = 0;
    ssize_t read;
    char *line = NULL;

    srand(time(NULL));

    fp = fopen("xmpp.list", "r");
    if(fp == NULL)
    {
        g_print("xmpp.list doesn't exist!\n");
        return -1;
    }

    while((read = getline(&line, &len, fp)) != -1)
    {
        line[strcspn(line, "\n")] = 0;

        char *pub = strtok(line, ":");
        char *tor = strtok(NULL, ":");
        int x;
        for(x = 1; x <= 100; x++)
            jab_register(pub, tor, 5222, 0);
        //break;
    }

    g_print("\n");

    fclose(fp);
    if(line)
        free(line);

    return 0;
}
