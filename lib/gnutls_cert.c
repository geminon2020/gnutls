/*
 *      Copyright (C) 2001,2002 Nikos Mavroyanopoulos
 *
 * This file is part of GNUTLS.
 *
 *  The GNUTLS library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public   
 *  License as published by the Free Software Foundation; either 
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

/* Some of the stuff needed for Certificate authentication is contained
 * in this file.
 */

#include <gnutls_int.h>
#include <gnutls_errors.h>
#include <auth_cert.h>
#include <gnutls_cert.h>
#include <libtasn1.h>
#include <gnutls_datum.h>
#include <gnutls_mpi.h>
#include <gnutls_global.h>
#include <x509_verify.h>
#include <gnutls_privkey.h>
#include <x509_extensions.h>
#include <gnutls_algorithms.h>
#include <gnutls_dh.h>
#include <gnutls_str.h>
#include <gnutls_state.h>
#include <gnutls_auth_int.h>
#include <gnutls_x509.h>
#include <gnutls_extra.h>

/* KX mappings to PK algorithms */
typedef struct {
	gnutls_kx_algorithm kx_algorithm;
	gnutls_pk_algorithm pk_algorithm;
} gnutls_pk_map;

/* This table maps the Key exchange algorithms to
 * the certificate algorithms. Eg. if we have
 * RSA algorithm in the certificate then we can
 * use GNUTLS_KX_RSA or GNUTLS_KX_DHE_RSA.
 */
static const gnutls_pk_map pk_mappings[] = {
	{GNUTLS_KX_RSA, GNUTLS_PK_RSA},
	{GNUTLS_KX_RSA_EXPORT, GNUTLS_PK_RSA},
	{GNUTLS_KX_DHE_RSA, GNUTLS_PK_RSA},
	{GNUTLS_KX_DHE_DSS, GNUTLS_PK_DSA},
	{0}
};

#define GNUTLS_PK_MAP_LOOP(b) \
        const gnutls_pk_map *p; \
                for(p = pk_mappings; p->kx_algorithm != 0; p++) { b ; }

#define GNUTLS_PK_MAP_ALG_LOOP(a) \
                        GNUTLS_PK_MAP_LOOP( if(p->kx_algorithm == kx_algorithm) { a; break; })


/* returns the gnutls_pk_algorithm which is compatible with
 * the given gnutls_kx_algorithm.
 */
gnutls_pk_algorithm _gnutls_map_pk_get_pk(gnutls_kx_algorithm kx_algorithm)
{
	gnutls_pk_algorithm ret = -1;

	GNUTLS_PK_MAP_ALG_LOOP(ret = p->pk_algorithm);
	return ret;
}

void _gnutls_free_cert(gnutls_cert cert)
{
	int i;

	for (i = 0; i < cert.params_size; i++) {
		_gnutls_mpi_release(&cert.params[i]);
	}

	gnutls_free_datum(&cert.signature);
	gnutls_free_datum(&cert.raw);

	return;
}

/**
  * gnutls_certificate_free_credentials - Used to free an allocated CERTIFICATE CREDENTIALS structure
  * @sc: is an &GNUTLS_CERTIFICATE_CREDENTIALS structure.
  *
  * This structure is complex enough to manipulate directly thus
  * this helper function is provided in order to free (deallocate)
  * the structure.
  **/
void gnutls_certificate_free_credentials(GNUTLS_CERTIFICATE_CREDENTIALS sc)
{
	int i, j;

	for (i = 0; i < sc->ncerts; i++) {
		for (j = 0; j < sc->cert_list_length[i]; j++) {
			_gnutls_free_cert(sc->cert_list[i][j]);
		}
		gnutls_free( sc->cert_list[i]);
	}

	gnutls_free(sc->cert_list_length);
	gnutls_free(sc->cert_list);

	for (j = 0; j < sc->x509_ncas; j++) {
		_gnutls_free_cert( sc->x509_ca_list[j]);
	}

	gnutls_free( sc->x509_ca_list);
	gnutls_free_datum( &sc->keyring);

	for (i = 0; i < sc->ncerts; i++) {
		_gnutls_free_private_key(sc->pkey[i]);
	}

	gnutls_free( sc->pkey);
	gnutls_free( sc->x509_rdn_sequence.data);

	gnutls_free( sc);
}


/**
  * gnutls_certificate_allocate_credentials - Used to allocate an x509 SERVER CREDENTIALS structure
  * @res: is a pointer to an &GNUTLS_CERTIFICATE_CREDENTIALS structure.
  *
  * This structure is complex enough to manipulate directly thus
  * this helper function is provided in order to allocate
  * the structure.
  **/
int gnutls_certificate_allocate_credentials(GNUTLS_CERTIFICATE_CREDENTIALS * res)
{
	*res = gnutls_calloc(1, sizeof(CERTIFICATE_CREDENTIALS_INT));

	if (*res == NULL)
		return GNUTLS_E_MEMORY_ERROR;

	(*res)->dh_params = &_gnutls_dh_default_params;

	return 0;
}


/* returns the KX algorithms that are supported by a
 * certificate. (Eg a certificate with RSA params, supports
 * GNUTLS_KX_RSA algorithm).
 * This function also uses the KeyUsage field of the certificate
 * extensions in order to disable unneded algorithms.
 */
int _gnutls_cert_supported_kx(const gnutls_cert * cert, gnutls_kx_algorithm ** alg,
			      int *alg_size)
{
	gnutls_kx_algorithm kx;
	int i;
	gnutls_pk_algorithm pk;
	gnutls_kx_algorithm kxlist[MAX_ALGOS];

	i = 0;
	for (kx = 0; kx < MAX_ALGOS; kx++) {
		pk = _gnutls_map_pk_get_pk(kx);
		if (pk == cert->subject_pk_algorithm) {
			if (cert->cert_type==GNUTLS_CRT_X509) {
				/* then check key usage */
				if (_gnutls_check_x509_key_usage(cert, kx) == 0) {
					kxlist[i] = kx;
					i++;
				}
			} else if ( cert->cert_type==GNUTLS_CRT_OPENPGP) {
				/* FIXME: something like key usage
				 * should be added
				 */
				kxlist[i] = kx;
				i++;
			}
		}
	}

	if (i==0) {
		gnutls_assert();
		return GNUTLS_E_INVALID_PARAMETERS;
	}

	*alg = gnutls_calloc(1, sizeof(gnutls_kx_algorithm) * i);
	if (*alg == NULL)
		return GNUTLS_E_MEMORY_ERROR;

	*alg_size = i;

	memcpy(*alg, kxlist, i * sizeof(gnutls_kx_algorithm));

	return 0;
}


/**
  * gnutls_certificate_server_set_request - Used to set whether to request a client certificate
  * @session: is an &gnutls_session structure.
  * @req: is one of GNUTLS_CERT_REQUEST, GNUTLS_CERT_REQUIRE
  *
  * This function specifies if we (in case of a server) are going
  * to send a certificate request message to the client. If 'req'
  * is GNUTLS_CERT_REQUIRE then the server will return an error if
  * the peer does not provide a certificate. If you do not
  * call this function then the client will not be asked to
  * send a certificate.
  **/
void gnutls_certificate_server_set_request(gnutls_session session,
					    gnutls_certificate_request req)
{
	session->internals.send_cert_req = req;
}

/**
  * gnutls_certificate_client_set_select_function - Used to set a callback while selecting the proper (client) certificate
  * @session: is a &gnutls_session structure.
  * @func: is the callback function
  *
  * The callback's function form is:
  * int (*callback)(gnutls_session, gnutls_datum *client_cert, int ncerts, gnutls_datum* req_ca_cert, int nreqs);
  *
  * 'client_cert' contains 'ncerts' gnutls_datum structures which hold
  * the raw certificates (DER for X.509 or binary for OpenPGP), of the
  * client.
  *
  * 'req_ca_cert', is only used in X.509 certificates. 
  * Contains a list with the CA names that the server considers trusted. 
  * Normaly we should send a certificate that is signed
  * by one of these CAs. These names are DER encoded. To get a more
  * meaningful value use the function gnutls_x509_extract_dn().
  *
  * This function specifies what we, in case of a client, are going
  * to do when we have to send a certificate. If this callback
  * function is not provided then gnutls will automaticaly try to
  * find an appropriate certificate to send.
  *
  * If the callback function is provided then gnutls will call it
  * once with NULL parameters. If the callback function returns
  * a positive or zero number then gnutls will attempt to automaticaly
  * choose the appropriate certificate. If gnutls fails to find an appropriate
  * certificate, then it will call the callback function again with the 
  * appropriate parameters.
  *
  * In case the callback returned a negative number then gnutls will
  * not attempt to choose the appropriate certificate and will call again
  * the callback function with the appropriate parameters, and rely
  * only to the return value of the callback function.
  *
  * The callback function should return the index of the certificate
  * choosen by the user. -1 indicates that the user
  * does not want to use client authentication.
  *
  * This function returns 0 on success.
  **/
void gnutls_certificate_client_set_select_function(gnutls_session session,
					     certificate_client_select_func
					     * func)
{
	session->internals.client_cert_callback = func;
}

/**
  * gnutls_certificate_server_set_select_function - Used to set a callback while selecting the proper (server) certificate
  * @session: is a &gnutls_session structure.
  * @func: is the callback function
  *
  * The callback's function form is:
  * int (*callback)(gnutls_session, gnutls_datum *server_cert, int ncerts);
  *
  * 'server_cert' contains 'ncerts' gnutls_datum structures which hold
  * the raw certificate (DER encoded in X.509) of the server. 
  *
  * This function specifies what we, in case of a server, are going
  * to do when we have to send a certificate. If this callback
  * function is not provided then gnutls will automaticaly try to
  * find an appropriate certificate to send. (actually send the first in the list)
  *
  * In case the callback returned a negative number then gnutls will
  * not attempt to choose the appropriate certificate and the caller function
  * will fail.
  *
  * The callback function will only be called once per handshake.
  * The callback function should return the index of the certificate
  * choosen by the server. -1 indicates an error.
  *
  **/
void gnutls_certificate_server_set_select_function(gnutls_session session,
					     certificate_server_select_func
					     * func)
{
	session->internals.server_cert_callback = func;
}

/* These are set by the gnutls_extra library's initialization function.
 */

OPENPGP_KEY_CREATION_TIME_FUNC _E_gnutls_openpgp_extract_key_creation_time = NULL;
OPENPGP_KEY_EXPIRATION_TIME_FUNC _E_gnutls_openpgp_extract_key_expiration_time = NULL;
OPENPGP_VERIFY_KEY_FUNC _E_gnutls_openpgp_verify_key = NULL;

/*-
  * _gnutls_openpgp_cert_verify_peers - This function returns the peer's certificate status
  * @session: is a gnutls session
  *
  * This function will try to verify the peer's certificate and return it's status (TRUSTED, INVALID etc.). 
  * Returns a negative error code in case of an error, or GNUTLS_E_NO_CERTIFICATE_FOUND if no certificate was sent.
  *
  -*/
int _gnutls_openpgp_cert_verify_peers(gnutls_session session)
{
	CERTIFICATE_AUTH_INFO info;
	const GNUTLS_CERTIFICATE_CREDENTIALS cred;
	gnutls_certificate_status verify;
	int peer_certificate_list_size;

	CHECK_AUTH(GNUTLS_CRD_CERTIFICATE, GNUTLS_E_INVALID_REQUEST);

	info = _gnutls_get_auth_info(session);
	if (info == NULL)
		return GNUTLS_E_INVALID_REQUEST;

	cred = _gnutls_get_cred(session->gnutls_key, GNUTLS_CRD_CERTIFICATE, NULL);
	if (cred == NULL) {
		gnutls_assert();
		return GNUTLS_E_INSUFICIENT_CREDENTIALS;
	}

	if (info->raw_certificate_list == NULL || info->ncerts == 0) {
		gnutls_assert();
		return GNUTLS_E_NO_CERTIFICATE_FOUND;
	}
	
	/* generate a list of gnutls_certs based on the auth info
	 * raw certs.
	 */
	peer_certificate_list_size = info->ncerts;

	if (peer_certificate_list_size != 1) {
		gnutls_assert();
		return GNUTLS_E_UNKNOWN_ERROR;
	}
	
	/* Verify certificate 
	 */
	if (_E_gnutls_openpgp_verify_key==NULL) {
		gnutls_assert();
		return GNUTLS_E_INIT_LIBEXTRA;
	}
	verify = _E_gnutls_openpgp_verify_key( cred->pgp_trustdb, &cred->keyring, &info->raw_certificate_list[0],
				      peer_certificate_list_size);

	if (verify < 0) {
		gnutls_assert();
		return GNUTLS_CERT_INVALID;
	}


	return verify;
}

/**
  * gnutls_certificate_verify_peers - This function returns the peer's certificate verification status
  * @session: is a gnutls session
  *
  * This function will try to verify the peer's certificate and return it's status (trusted, invalid etc.). 
  * However you must also check the peer's name in order to check if the verified certificate belongs to the 
  * actual peer. 
  *
  * The return value should be one or more of the gnutls_certificate_status 
  * enumerated elements bitwise or'd. This is the same as
  * gnutls_x509_verify_certificate().
  *
  **/
int gnutls_certificate_verify_peers(gnutls_session session)
{
	CERTIFICATE_AUTH_INFO info;

	CHECK_AUTH(GNUTLS_CRD_CERTIFICATE, GNUTLS_E_INVALID_REQUEST);

	info = _gnutls_get_auth_info(session);
	if (info == NULL) {
		gnutls_assert();
		return GNUTLS_E_NO_CERTIFICATE_FOUND;
	}
	
	if (info->raw_certificate_list == NULL || info->ncerts == 0)
		return GNUTLS_E_NO_CERTIFICATE_FOUND;

	switch( gnutls_certificate_type_get( session)) {
		case GNUTLS_CRT_X509:
			return _gnutls_x509_cert_verify_peers( session);
		case GNUTLS_CRT_OPENPGP:
			return _gnutls_openpgp_cert_verify_peers( session);
		default:
			return GNUTLS_E_INVALID_REQUEST;
	}
}

/**
  * gnutls_certificate_expiration_time_peers - This function returns the peer's certificate expiration time
  * @session: is a gnutls session
  *
  * This function will return the peer's certificate expiration time.
  *
  * Returns (time_t) -1 on error.
  *
  **/
time_t gnutls_certificate_expiration_time_peers(gnutls_session session)
{
	CERTIFICATE_AUTH_INFO info;

	CHECK_AUTH(GNUTLS_CRD_CERTIFICATE, GNUTLS_E_INVALID_REQUEST);

	info = _gnutls_get_auth_info(session);
	if (info == NULL) {
		gnutls_assert();
		return (time_t) -1;
	}
	
	if (info->raw_certificate_list == NULL || info->ncerts == 0) {
		gnutls_assert();
		return (time_t) -1;
	}

	switch( gnutls_certificate_type_get( session)) {
		case GNUTLS_CRT_X509:
			return gnutls_x509_extract_certificate_expiration_time(
				&info->raw_certificate_list[0]);
		case GNUTLS_CRT_OPENPGP:
			if (_E_gnutls_openpgp_extract_key_expiration_time==NULL)
				return (time_t)-1;
			return _E_gnutls_openpgp_extract_key_expiration_time(
				&info->raw_certificate_list[0]);
		default:
			return (time_t)-1;
	}
}

/**
  * gnutls_certificate_activation_time_peers - This function returns the peer's certificate activation time
  * @session: is a gnutls session
  *
  * This function will return the peer's certificate activation time.
  * This is the creation time for openpgp keys.
  *
  * Returns (time_t) -1 on error.
  *
  **/
time_t gnutls_certificate_activation_time_peers(gnutls_session session)
{
	CERTIFICATE_AUTH_INFO info;

	CHECK_AUTH(GNUTLS_CRD_CERTIFICATE, GNUTLS_E_INVALID_REQUEST);

	info = _gnutls_get_auth_info(session);
	if (info == NULL) {
		gnutls_assert();
		return (time_t) -1;
	}
	
	if (info->raw_certificate_list == NULL || info->ncerts == 0) {
		gnutls_assert();
		return (time_t) -1;
	}

	switch( gnutls_certificate_type_get( session)) {
		case GNUTLS_CRT_X509:
			return gnutls_x509_extract_certificate_activation_time(
				&info->raw_certificate_list[0]);
		case GNUTLS_CRT_OPENPGP:
			if (_E_gnutls_openpgp_extract_key_creation_time==NULL)
				return (time_t)-1;
			return _E_gnutls_openpgp_extract_key_creation_time(
				&info->raw_certificate_list[0]);
		default:
			return (time_t)-1;
	}
}

