#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define ROSTER_TYPE_ROOM 0
#define ROSTER_TYPE_CHAT 1

extern GMainContext *main_context;

LmConnection *xmpp_connect(gchar *jpubserv, gchar *jconserv, gint jconport,
    gchar *jid, gchar *jpass, gchar *jresource, gboolean dossl);

void xmpp_disconnect(LmConnection *lconnection);

void xmpp_addjid(gchar *jid, LmConnection *lconnection);

void xmpp_deljid(gchar *jid, LmConnection *lconnection);

void xmpp_sendmsg(gchar *jid, gchar *subj, gchar *body, LmConnection *lconnection);

void xmpp_register_rand(gchar *pubserv, gchar *conserv, gchar *conport,
    gchar *authsfile, LmConnection *lconnection);

#endif
