/*
 *      Copyright (C) 2001 Nikos Mavroyanopoulos
 *
 * This file is part of GNUTLS.
 *
 * GNUTLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUTLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <defines.h>
#include "gnutls_int.h"
#include "gnutls_errors.h"
#include "gnutls_auth.h"

/* The functions here are used in order for authentication algorithms
 * to be able to retrieve the needed credentials eg public and private
 * key etc.
 */

/* This clears the whole linked list */
int gnutls_clear_creds( GNUTLS_STATE state) {
	AUTH_CRED * ccred, *ncred;
	
	if (state->gnutls_internals.cred!=NULL) { /* begining of the list */
		ccred = state->gnutls_internals.cred;
		while(ccred!=NULL) {
			ncred = ccred->next;
			if (ccred!=NULL) gnutls_free(ccred);
			ccred = ncred;
		}
		state->gnutls_internals.cred = NULL;
	}

	return 0;
}

/* 
 * This creates a linked list of the form:
 * { algorithm, credentials, pointer to next }
 */
int gnutls_set_kx_cred( GNUTLS_STATE state, int kx, void* cred) {
	AUTH_CRED * ccred, *pcred;
	int exists=0;	
	
	if (state->gnutls_internals.cred==NULL) { /* begining of the list */
		
		state->gnutls_internals.cred = gnutls_malloc(sizeof(AUTH_CRED));
		if (state->gnutls_internals.cred == NULL) return GNUTLS_E_MEMORY_ERROR;
		
		state->gnutls_internals.cred->credentials = cred;
		state->gnutls_internals.cred->next = NULL;
		state->gnutls_internals.cred->algorithm = kx;
	} else {
		ccred = state->gnutls_internals.cred;
		while(ccred!=NULL) {
			if (ccred->algorithm==kx) {
				exists=1;
				break;
			}
			pcred = ccred;
			ccred = ccred->next;
		}
		
		if (exists==0) { /* new entry */
			pcred->next = gnutls_malloc(sizeof(AUTH_CRED));
			if (pcred->next == NULL) return GNUTLS_E_MEMORY_ERROR;
		
			ccred = pcred->next;
			ccred->credentials = cred;
			ccred->next = NULL;
			ccred->algorithm = kx;
		} else { /* modify existing entry */
			ccred->credentials = cred;
		}
	}

	return 0;
}

/* 
 * This returns an item from the linked list
 */
AUTH_CRED *gnutls_get_kx_cred( GNUTLS_STATE state, int kx) {
	AUTH_CRED * ccred;
	
	ccred = state->gnutls_internals.cred;
	while(ccred!=NULL) {
		if (ccred->algorithm==kx) {
			break;
		}
		ccred = ccred->next;
	}
	if (ccred==NULL) return NULL;
			
	return ccred->credentials;
}
