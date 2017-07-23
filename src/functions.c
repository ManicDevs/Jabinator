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

gchar *jid;
gchar *jpass;
gchar *dynresource = NULL;

LmSSLResponse ssl_cb(LmSSL *ssl, LmSSLStatus status, gpointer ud)
{
    return LM_SSL_RESPONSE_CONTINUE;
}

LmHandlerResult handle_iq(LmMessageHandler *h, LmConnection *c, LmMessage *m, gpointer ud)
{
    return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

static void connection_close_cb(LmConnection *lconnection, LmDisconnectReason reason, gpointer ud)
{
    gchar *str;

    switch(reason)
    {
        case LM_DISCONNECT_REASON_OK:
            str = "LM_DISCONNECT_REASON_OK";
        break;

        case LM_DISCONNECT_REASON_PING_TIME_OUT:
            str = "LM_DISCONNECT_REASON_PING_TIME_OUT";
        break;

        case LM_DISCONNECT_REASON_HUP:
            str = "LM_DISCONNECT_REASON_HUP";
        break;

        case LM_DISCONNECT_REASON_ERROR:
            str = "LM_DISCONNECT_REASON_ERROR";
        break;

        case LM_DISCONNECT_REASON_UNKNOWN:
        default:
            str = "LM_DISCONNECT_REASON_UNKNOWN";
        break;
    }

    if(!reason)
        g_print("Disconnected!\n");
    else
        g_print("Disconnected, reason: %d->%s", reason, str);
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

gboolean xmpp_is_online(LmConnection *lconnection)
{
    if(lconnection && lm_connection_is_authenticated(lconnection))
        return TRUE;
    else
        return FALSE;
}

LmConnection *xmpp_connect(gchar *jpubserv, gchar *jconserv, gint jconport,
    gchar *jid, gchar *jpass, gchar *jresource, gboolean dossl)
{
    LmSSL *ssl;
    LmConnection *lconnection;
    LmMessageHandler *handler;
    GError *error;
    gchar *jidat;

    if(!dossl)
        dossl = FALSE;

    if(!jid)
    {
        g_print("JID is empty\n");
        return NULL;
    }

    if(!jpass)
    {
        g_print("JPASS is empty\n");
        return NULL;
    }

    lconnection = lm_connection_new_with_context(jconserv, main_context);
    lm_connection_set_keep_alive_rate(lconnection, 30);
    lm_connection_set_disconnect_function(lconnection, connection_close_cb, NULL, NULL);

    handler = lm_message_handler_new(handle_iq, NULL, NULL);
    lm_connection_register_message_handler(lconnection, handler, LM_MESSAGE_TYPE_IQ,
        LM_HANDLER_PRIORITY_NORMAL);
    lm_message_handler_unref(handler);

    if(!jconport)
        jconport = (!lm_ssl_is_supported() ? LM_CONNECTION_DEFAULT_PORT :
            LM_CONNECTION_DEFAULT_PORT_SSL);

    lm_connection_set_port(lconnection, jconport);

    g_print("Connecting to server %s:%d", jconserv, jconport);

    if(!jresource)
        jresource = "NULL";

    srandom(time(NULL));

    gint tab[2];
    tab[0] = (gint)(0xffff * (random() / (RAND_MAX + 1.0)));
    tab[1] = (gint)(0xffff * (random() / (RAND_MAX + 1.0)));
    dynresource = g_strdup_printf("%s.%04x%04x", jresource, tab[0], tab[1]);

    jresource = dynresource;

    g_print(" with resource %s\n", jresource);

    jidat = g_strdup_printf("%s@%s", jid, jpubserv);

    lm_connection_set_jid(lconnection, jidat);

    g_free(jidat);

    if(lm_ssl_is_supported() && dossl)
    {
        g_print("Setting SSL\n");
        ssl = lm_ssl_new(NULL, ssl_cb, NULL, NULL);
        lm_connection_set_ssl(lconnection, ssl);
        lm_ssl_unref(ssl);
    }

    if(!lm_connection_open_and_block(lconnection, &error))
    {
        g_free(dynresource);
        g_print("Failed to open connection: %s\n", error->message);
        return NULL;
    }

    if(!lm_connection_authenticate_and_block(lconnection, jid, jpass, dynresource, &error))
    {
        g_free(dynresource);
        g_print("Failed to authenticate: %s\n", error->message);
        return NULL;
    }

    g_free(dynresource);

    if(!lm_connection_is_open(lconnection))
        return NULL;

    return lconnection;
}

void xmpp_disconnect(LmConnection *lconnection)
{
    if(!lconnection)
        return;

    if(lm_connection_is_open(lconnection))
        lm_connection_close(lconnection, NULL);

    lm_connection_unref(lconnection);
}

void xmpp_send_s10n(gchar *jid, LmMessageSubType type, LmConnection *lconnection)
{
    LmMessage *m;

    if(G_UNLIKELY(!jid || !*jid))
    {
        g_print("Empty JID\n");
        return;
    }

    m = lm_message_new_with_sub_type(jid, LM_MESSAGE_TYPE_PRESENCE, type);
    lm_connection_send(lconnection, m, NULL);
    lm_message_unref(m);
}

void xmpp_addjid(gchar *jid, LmConnection *lconnection)
{
    LmMessage *miq;
    LmMessageNode *query, *query2;
    LmMessageHandler *handler;

    if(!xmpp_is_online(lconnection))
    {
        g_print("Not online!\n");
        return;
    }

    miq = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
    query = lm_message_node_add_child(miq->node, "query", NULL);
    lm_message_node_set_attribute(query, "xmlns", IQ_ROSTER);
    query2 = lm_message_node_add_child(query, "item", NULL);
    lm_message_node_set_attribute(query2, "jid", jid);
    lm_message_node_set_attribute(query2, "name", jid);

    handler = lm_message_handler_new(handle_iq, NULL, FALSE);
    lm_connection_send_with_reply(lconnection, miq, handler, NULL);
    lm_message_handler_unref(handler);
    lm_message_unref(miq);

    xmpp_send_s10n(jid, LM_MESSAGE_SUB_TYPE_SUBSCRIBE, lconnection);
}

void xmpp_deljid(gchar *jid, LmConnection *lconnection)
{
    LmMessage *miq;
    LmMessageNode *query, *query2;
    LmMessageHandler *handler;

    if(!xmpp_is_online(lconnection))
        return;

    // cancel "from"
    xmpp_send_s10n(jid, LM_MESSAGE_SUB_TYPE_UNSUBSCRIBED, lconnection);
    // cancel "to"
    xmpp_send_s10n(jid, LM_MESSAGE_SUB_TYPE_UNSUBSCRIBE, lconnection);

    miq = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ,
        LM_MESSAGE_SUB_TYPE_SET);
    query = lm_message_node_add_child(miq->node, "query", NULL);
    lm_message_node_set_attribute(query, "xmlns", IQ_ROSTER);
    query2 = lm_message_node_add_child(query, "item", NULL);
    lm_message_node_set_attribute(query2, "jid", jid);

    handler = lm_message_handler_new(handle_iq, NULL, FALSE);
    lm_connection_send_with_reply(lconnection, miq, handler, NULL);
    lm_message_handler_unref(handler);
    lm_message_unref(miq);
}

void xmpp_sendmsg(gchar *jid, gchar *subj, gchar *body, LmConnection *lconnection)
{
    LmMessage *m;
    GError *error;

    if(!xmpp_is_online(lconnection))
    {
        g_print("Not online!\n");
        return;
    }

    m = lm_message_new(jid, LM_MESSAGE_TYPE_MESSAGE);
    lm_message_node_add_child(m->node, "body", body);
    lm_message_node_add_child(m->node, "subject", subj);

    if(!lm_connection_send(lconnection, m, &error))
        g_printerr("Failed to send message:'%s'\n", lm_message_node_to_string(m->node));
    else
        g_print("Sent message:'%s'\n", lm_message_node_to_string(m->node));

    lm_message_unref(m);
}

void xmpp_register(gchar *pubserv, gchar *conserv, gchar *conport,
    gchar *jid, gchar *jpass, gchar *authsfile, LmConnection *lconnection)
{
    FILE            *fp;
    LmMessage       *miq, *reply;
    LmMessageNode   *query, *node;
    GError          *error;

    g_print("%s|%s:%s|%s|%s\n", pubserv, conserv, conport, jid, jpass);

    jid = g_strdup_printf("%s@%s", jid, pubserv);
    lm_connection_set_jid(lconnection, jid);

    miq = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
    query = lm_message_node_add_child(miq->node, "query", NULL);
    lm_message_node_set_attributes(query, "xmlns", IQ_REGISTER, NULL);
    lm_message_node_add_child(query, "username", jid);
    lm_message_node_add_child(query, "password", jpass);

    reply = lm_connection_send_with_reply_and_block(lconnection, miq, &error);
    if(!reply)
    {
        g_free(jid);
        g_print("Failed to send registration: %s\n", error->message);
        return;
    }

    switch(lm_message_get_sub_type(reply))
    {
        case LM_MESSAGE_SUB_TYPE_RESULT:
            fp = fopen(authsfile, "a");
            if(fp == NULL)
            {
                g_print("Failed to open %s\n", authsfile);
                return;
            }
            fprintf(fp, "%s|%s:%s|%s|%s\n", pubserv, conserv, conport, jid, jpass);
            fclose(fp);
            g_print("Succeeded in registering account '%s@%s'\n", jid, pubserv);
        break;

        case LM_MESSAGE_SUB_TYPE_ERROR:
            g_print("Failed to register account '%s@%s' due to: ", jid, pubserv);
            node = lm_message_node_find_child(reply->node, "error");
            if(node)
                g_print("%s\n", lm_message_node_get_value(node));
            else
                g_print("Unknown error\n");
        break;

        default:
            g_print("Unknown error\n");
        break;
    }

    g_free(jid);
}

void xmpp_register_rand(gchar *pubserv, gchar *conserv, gchar *conport,
    gchar *authsfile, LmConnection *lconnection)
{
    gchar randjid[8] = "";
    gchar randjpass[8] = "";

    rand_str(randjid, 7);

    srandom(time(NULL));

    rand_str(randjpass, 7);
    jpass = randjpass;

    xmpp_register(pubserv, conserv, conport, randjid, jpass, authsfile, lconnection);
}
