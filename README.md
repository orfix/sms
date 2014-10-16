#Simple Messaging System (SMS)
A messaging system, written in C for unix-like systems. Divided into two agnostic IP applications, a multi-client server and a client, the communication is done through a homemade protocol.

## Sample Run
```
./server 1111 &
./client ipv4 localhost 1111
-- Copyright (C) 2010 Mounir Orfi <mounir.orfi@gmail.com>
-- Welcome to SMC version 0.1
mail>
mail>
mail> help

Available commands:
HELP            Show this help message
LOGIN           Log in to your account
LOGOUT          Close the connection
HEADS           Get the mail headers
READ            Retreive a mail
DELETE          Delete a mail
SEND            Send a mail to a registered member
REGISTER        Become a registered member

mail> login mounir 12345
+ Welcome mounir orfi
mail> heads 0
ID      FROM            DATE                    SUBJECT
--------------------------------------------------------------------------------
45      mounir          2010-05-30 00:46:10     Royal-O-Fish
41      amine           2010-05-30 00:44:49     Le ptit'Wra
38      amine           2010-05-30 00:43:59     Le M de McDonald's
37      amine           2010-05-30 00:43:34     Fajita
30      linus           2010-05-30 00:42:56     280 Moutarde

mail> heads 1
ID      FROM            DATE                    SUBJECT
--------------------------------------------------------------------------------
29      linus           2010-05-30 00:42:55     280 Classique
26      amine           2010-05-30 00:41:57     Hamburger au boeuf Angus avec cheddar et bacon

mail> help read

Usage      : READ <mail id>
Description: Retreive a mail.

mail> read 4
- Invalid mail ID
mail> read 45

FROM   : mounir
SUBJECT: Royal-O-Fish

couronne avec sésame, sauce tomate avec du persil, laitue émincée, tranche de tomate (x1), patty Filet, talon.

mail> logout
+ Bye!
```

