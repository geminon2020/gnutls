/*
 * Copyright (C) 2004, 2005, 2006, 2007 Free Software Foundation
 *
 * Author: Simon Josefsson
 *
 * This file is part of GNUTLS.
 *
 * GNUTLS is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNUTLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNUTLS; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* Parts copied from GnuTLS example programs. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <gnutls/gnutls.h>
#include <gnutls/openpgp.h>

#include "utils.h"

#include "ex-session-info.c"
#include "ex-x509-info.c"

pid_t child;

static void
tls_log_func (int level, const char *str)
{
  fprintf (stderr, "%s |<%d>| %s", child ? "server" : "client", level, str);
}

/* A very basic TLS client, with anonymous authentication.
 */

#define MAX_BUF 1024
#define MSG "Hello TLS"

/* Connects to the peer and returns a socket
 * descriptor.
 */
int
tcp_connect (void)
{
  const char *PORT = "5556";
  const char *SERVER = "127.0.0.1";
  int err, sd;
  struct sockaddr_in sa;

  /* connects to server
   */
  sd = socket (AF_INET, SOCK_STREAM, 0);

  memset (&sa, '\0', sizeof (sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons (atoi (PORT));
  inet_pton (AF_INET, SERVER, &sa.sin_addr);

  err = connect (sd, (struct sockaddr *) &sa, sizeof (sa));
  if (err < 0)
    {
      fprintf (stderr, "Connect error\n");
      exit (1);
    }

  return sd;
}

/* closes the given socket descriptor.
 */
void
tcp_close (int sd)
{
  shutdown (sd, SHUT_RDWR);	/* no more receptions */
  close (sd);
}

void
client (void)
{
  int ret, sd, ii;
  gnutls_session_t session;
  char buffer[MAX_BUF + 1];
  gnutls_certificate_credentials_t xcred;
  const char* seckey, *pubkey;

  gnutls_global_init ();

  gnutls_global_set_log_function (tls_log_func);
  gnutls_global_set_log_level (4711);

  gnutls_certificate_allocate_credentials (&xcred);

  /* sets the trusted cas file
   */
  success ("Setting key files...\n");

  pubkey = "../doc/credentials/openpgp/cli_pub.asc";
  seckey = "../doc/credentials/openpgp/cli_sec.asc";
  ret = gnutls_certificate_set_openpgp_key_file (xcred, pubkey, seckey, GNUTLS_OPENPGP_FMT_BASE64);
  if (ret < 0)
    {
      fail ("Could not set key files...\n");
    }

  /* Initialize TLS session
   */
  gnutls_init (&session, GNUTLS_CLIENT);

  /* Use default priorities */
  gnutls_set_default_priority (session);

  /* put the x509 credentials to the current session
   */
  gnutls_credentials_set (session, GNUTLS_CRD_CERTIFICATE, xcred);

  /* connect to the peer
   */
  success ("Connecting...\n");
  sd = tcp_connect ();

  gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) sd);

  /* Perform the TLS handshake
   */
  ret = gnutls_handshake (session);

  if (ret < 0)
    {
      fail ("client: Handshake failed\n");
      gnutls_perror (ret);
      goto end;
    }
  else
    {
      success ("client: Handshake was completed\n");
    }

  success ("client: TLS version is: %s\n",
	   gnutls_protocol_get_name (gnutls_protocol_get_version (session)));

  /* see the Getting peer's information example */
  print_info(session);

  gnutls_record_send (session, MSG, strlen (MSG));

  ret = gnutls_record_recv (session, buffer, MAX_BUF);
  if (ret == 0)
    {
      success ("client: Peer has closed the TLS connection\n");
      goto end;
    }
  else if (ret < 0)
    {
      fail ("client: Error: %s\n", gnutls_strerror (ret));
      goto end;
    }

  printf ("- Received %d bytes: ", ret);
  for (ii = 0; ii < ret; ii++)
    {
      fputc (buffer[ii], stdout);
    }
  fputs ("\n", stdout);

  gnutls_bye (session, GNUTLS_SHUT_RDWR);

end:

  tcp_close (sd);

  gnutls_deinit (session);

  gnutls_certificate_free_credentials (xcred);

  gnutls_global_deinit ();
}

/* This is a sample TLS 1.0 echo server, using X.509 authentication.
 */

#define SA struct sockaddr
#define MAX_BUF 1024
#define PORT 5556		/* listen to 5556 port */
#define DH_BITS 1024

/* These are global */
gnutls_certificate_credentials_t pgp_cred;

gnutls_session_t
initialize_tls_session (void)
{
  gnutls_session_t session;

  gnutls_init (&session, GNUTLS_SERVER);

  /* avoid calling all the priority functions, since the defaults
   * are adequate.
   */
  gnutls_set_default_priority (session);

  gnutls_credentials_set (session, GNUTLS_CRD_CERTIFICATE, pgp_cred);

  /* request client certificate if any.
   */
  gnutls_certificate_server_set_request (session, GNUTLS_CERT_REQUEST);

  gnutls_dh_set_prime_bits (session, DH_BITS);

  return session;
}

static gnutls_dh_params_t dh_params;

static int
generate_dh_params (void)
{
  const gnutls_datum_t p3 = { pkcs3, strlen (pkcs3) };
  /* Generate Diffie Hellman parameters - for use with DHE
   * kx algorithms. These should be discarded and regenerated
   * once a day, once a week or once a month. Depending on the
   * security requirements.
   */
  gnutls_dh_params_init (&dh_params);
  return gnutls_dh_params_import_pkcs3 (dh_params, &p3, GNUTLS_X509_FMT_PEM);
}

int err, listen_sd, i;
int sd, ret;
struct sockaddr_in sa_serv;
struct sockaddr_in sa_cli;
int client_len;
char topbuf[512];
gnutls_session_t session;
char buffer[MAX_BUF + 1];
int optval = 1;

void
server_start (void)
{
  const char* pubkey, *seckey;
  /* this must be called once in the program
   */
  gnutls_global_init ();

  gnutls_global_set_log_function (tls_log_func);
  gnutls_global_set_log_level (4711);

  gnutls_certificate_allocate_credentials (&pgp_cred);

  pubkey = "../doc/credentials/openpgp/pub.asc";
  seckey = "../doc/credentials/openpgp/sec.asc";
  err = gnutls_certificate_set_openpgp_key_file2 (pgp_cred, pubkey, seckey, "auto", GNUTLS_OPENPGP_FMT_BASE64);
  if (err < 0)
    {
      fail ("Could not set key files...\n");
    }

  success ("Launched, setting DH parameters...\n");

  generate_dh_params ();

  gnutls_certificate_set_dh_params (pgp_cred, dh_params);

  /* Socket operations
   */
  listen_sd = socket (AF_INET, SOCK_STREAM, 0);
  if (err == -1)
    {
      perror ("socket");
      fail ("server: socket failed\n");
      return;
    }

  memset (&sa_serv, '\0', sizeof (sa_serv));
  sa_serv.sin_family = AF_INET;
  sa_serv.sin_addr.s_addr = INADDR_ANY;
  sa_serv.sin_port = htons (PORT);	/* Server Port number */

  setsockopt (listen_sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (int));

  err = bind (listen_sd, (SA *) & sa_serv, sizeof (sa_serv));
  if (err == -1)
    {
      perror ("bind");
      fail ("server: bind failed\n");
      return;
    }

  err = listen (listen_sd, 1024);
  if (err == -1)
    {
      perror ("listen");
      fail ("server: listen failed\n");
      return;
    }

  success ("server: ready. Listening to port '%d'.\n", PORT);
}

void
server (void)
{
  client_len = sizeof (sa_cli);

  session = initialize_tls_session ();

  sd = accept (listen_sd, (SA *) & sa_cli, &client_len);

  success ("server: connection from %s, port %d\n",
	   inet_ntop (AF_INET, &sa_cli.sin_addr, topbuf,
		      sizeof (topbuf)), ntohs (sa_cli.sin_port));

  gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) sd);
  ret = gnutls_handshake (session);
  if (ret < 0)
    {
      close (sd);
      gnutls_deinit (session);
      fail ("server: Handshake has failed (%s)\n\n", gnutls_strerror (ret));
      return;
    }
  success ("server: Handshake was completed\n");

  success ("server: TLS version is: %s\n",
	   gnutls_protocol_get_name (gnutls_protocol_get_version (session)));

  /* see the Getting peer's information example */
  print_info(session);

  i = 0;
  for (;;)
    {
      bzero (buffer, MAX_BUF + 1);
      ret = gnutls_record_recv (session, buffer, MAX_BUF);

      if (ret == 0)
	{
	  success ("server: Peer has closed the GNUTLS connection\n");
	  break;
	}
      else if (ret < 0)
	{
	  fail ("server: Received corrupted data(%d). Closing...\n", ret);
	  break;
	}
      else if (ret > 0)
	{
	  /* echo data back to the client
	   */
	  gnutls_record_send (session, buffer, strlen (buffer));
	}
    }
  /* do not wait for the peer to close the connection.
   */
  gnutls_bye (session, GNUTLS_SHUT_WR);

  close (sd);
  gnutls_deinit (session);

  close (listen_sd);

  gnutls_certificate_free_credentials (pgp_cred);

  gnutls_global_deinit ();

  success ("server: finished\n");
}


void
doit (void)
{
  server_start ();
  if (error_count)
    return;

  child = fork ();
  if (child < 0)
    {
      perror ("fork");
      fail ("fork");
      return;
    }

  if (child)
    {
      int status;
      /* parent */
      server ();
      wait (&status);
    }
  else
    client ();
}
