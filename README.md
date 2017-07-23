# Jabinator
Where XMPP gets it's JAB

# Information
In the working directory, i.e. bin/ - When it's created after running "make" you'll need the below files!

--- xmpp.list - Generated via yourself, needed for ./jabinator-regflood

* Format: pubserv|connectserv:connectport

--- auth.list - Generated via ./jabinator-regflood, needed for ./jabinator-addflood

* Format: pubserv|connectserv:port|user|pass

# Usage
## jabinator-regflood
$ ./jabinator-regflood --help
```
Usage:
  jabinator-regflood [OPTION?] - Jabinator - Registration flooder

Help Options:
  -h, --help         Show help options

Application Options:
  -x, --xmppfile     XMPPs file list for input (e.g. xmpp.list)
  -o, --outfile      Authentications file list to output (e.g. auth.list)
  -t, --threads      Number of threads to use [default=1]
  -c, --cycles       Number of cycles to register accounts [default=1]
```


## jabinator-addflood
```
$ ./jabinator-addflood --help

Usage:
  jabinator-addflood [OPTION?] - Jabinator - Add flooder

Help Options:
  -h, --help          Show help options

Application Options:
  -a, --auths         Authentications file list (e.g. auth.list)
  -t, --threads       Number of threads to use [default=1]
  -R, --recipient     Recipient to send the message to (e.g. user@server.org)
  -r, --resource      Resource to connect with [default=NULL]
```


## jabinator-msgflood
$ ./jabinator-msgflood --help
```
Usage:
  jabinator-msgflood [OPTION?] - Jabinator - Message flooder

Help Options:
  -h, --help          Show help options

Application Options:
  -a, --auths         Authentications file list (e.g. auth.list)
  -t, --threads       Number of threads to use [default=1]
  -R, --recipient     Recipient to send the message to (e.g. user@server.org)
  -s, --subject       Subject line to send [default=You need this JAB!]
  -m, --message       Message to send to recipient [default=Don't be a fool, take your meds!]
  -r, --resource      Resource to connect with [default=NULL]
```


