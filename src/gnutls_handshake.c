#include <defines.h>
#include "gnutls_int.h"
#include "gnutls_errors.h"
#include "debug.h"
#include "gnutls_compress.h"
#include "gnutls_plaintext.h"
#include "gnutls_cipher.h"
#include "gnutls_buffers.h"
#include "gnutls_handshake.h"
#include "gnutls_num.h"

#ifdef DEBUG
#define ERR(x, y) fprintf(stderr, "GNUTLS Error: %s (%d)\n", x,y)
#else
#define ERR(x, y)
#endif

int SelectSuite( opaque ret[2], char* data, int datalen) {
int x, pos=0, i,j;
GNUTLS_CipherSuite *ciphers;

	x = _gnutls_supported_ciphersuites(&ciphers);
	memset( ret, '\0', sizeof(GNUTLS_CipherSuite));
	
	for ( j=0;j<datalen;j+=2) {
		for (i=0;i<x;i++) {
			if ( memcmp( &ciphers[i].CipherSuite, &data[j], 2) == 0) {
				memmove( ret, &ciphers[i].CipherSuite, 2);
				gnutls_free(ciphers);
				return 0;
			}
		}
	}
	
	
	gnutls_free(ciphers);
	return GNUTLS_E_UNKNOWN_CIPHER;

}

int SelectCompMethod( CompressionMethod* ret, char* data, int datalen) {
int x, pos=0, i,j;
CompressionMethod *ciphers;

	x = _gnutls_supported_compression_methods(&ciphers);
	memset( ret, '\0', sizeof(CompressionMethod));
	
	for ( j=0;j<datalen;j++) {
		for (i=0;i<x;i++) {
			if ( memcmp( &ciphers[i], &data[j], 1) == 0) {
				memmove( ret, &ciphers[i], 1);
				gnutls_free(ciphers);
				return 0;
			}
		}
	}
	
	
	gnutls_free(ciphers);
	return GNUTLS_E_UNKNOWN_COMPRESSION_ALGORITHM;

}


#define SUPPORTED_CIPHERSUITES 1
int _gnutls_supported_ciphersuites(GNUTLS_CipherSuite **ciphers) {

	int i;

	*ciphers = gnutls_malloc( SUPPORTED_CIPHERSUITES * sizeof(GNUTLS_CipherSuite));

	for (i=0;i<SUPPORTED_CIPHERSUITES;i++) {
		(*ciphers)[i].CipherSuite[0] = 0x00;
	}

	/* GNUTLS_DH_anon_WITH_3DES_EDE_CBC_SHA */
//	(*ciphers)[0].CipherSuite[1] = 0x1B;

	/* GNUTLS_NULL_WITH_NULL_NULL */
	(*ciphers)[0].CipherSuite[1] = 0x0;

	return SUPPORTED_CIPHERSUITES;
}


#define SUPPORTED_COMPRESSION_METHODS 1
int _gnutls_supported_compression_methods(CompressionMethod **comp) {

	int i;

	*comp = gnutls_malloc( SUPPORTED_COMPRESSION_METHODS * sizeof(CompressionMethod));

/* NULL Compression */
	(*comp)[0] = COMPRESSION_NULL;

	return SUPPORTED_COMPRESSION_METHODS;
}

int _gnutls_send_handshake(int cd, GNUTLS_STATE state, void* i_data, uint32 i_datasize, HandshakeType type) {
	int ret;
	uint8* data; 
	uint24 length;
	uint32 datasize;
	int pos=0;
	
#ifdef WORDS_BIGENDIAN
	datasize = i_datasize;
#else 
	datasize = byteswap32(i_datasize);
#endif	
	
	length = uint32touint24( datasize);
//	length.pint[0] = ((uint8*)&datasize)[1];
//	length.pint[1] = ((uint8*)&datasize)[2];
//	length.pint[2] = ((uint8*)&datasize)[3];

	i_datasize += 4;
	data = gnutls_malloc( i_datasize);
	memmove( &data[pos++], &type, 1);
	memmove( &data[pos++], &length.pint[0], 1);
	memmove( &data[pos++], &length.pint[1], 1);
	memmove( &data[pos++], &length.pint[2], 1);
	if (i_datasize > 0) memmove( &data[pos], i_data, i_datasize-4);

	ret = gnutls_send_int( cd, state, GNUTLS_HANDSHAKE, data, i_datasize);
	
	return ret;
}

int _gnutls_recv_handshake( int cd, GNUTLS_STATE state, void* data, uint32 datasize) {
	int ret;
	uint32 length32=0;
	int pos=0;
	char *dataptr=data;
	uint24 num;

	num.pint[0] = dataptr[1];
	num.pint[1] = dataptr[2];
	num.pint[2] = dataptr[3];
	length32 = uint24touint32( num);

#ifndef WORDS_BIGENDIAN	
	length32 = byteswap32(length32);
#endif


	switch(dataptr[0]) {
		case GNUTLS_CLIENT_HELLO:
		case GNUTLS_SERVER_HELLO:
			ret = _gnutls_recv_hello( cd, state, &dataptr[4], length32, NULL, 0);
			break;
	}

	return ret;
}

int _gnutls_send_hello_request(int cd, GNUTLS_STATE state) {
	return _gnutls_send_handshake( cd, state, NULL, 0, GNUTLS_HELLO_REQUEST);
}


int _gnutls_send_hello(int cd, GNUTLS_STATE state, opaque* SessionID, uint8 SessionIDLen) {
	char* rand;
	char *data=NULL;
	uint8 session_id_len, z;
	uint32 cur_time;
	int pos=0;
	GNUTLS_CipherSuite* cipher_suites;
	CompressionMethod* compression_methods;
	int i, datalen, ret=0;
	uint16 x;
	
	session_id_len = SessionIDLen;
	if (SessionID==NULL) session_id_len=0;
	rand=gcry_random_bytes( 28, GCRY_STRONG_RANDOM);
	
	if (state->security_parameters.entity == GNUTLS_CLIENT) {

		datalen = 2 + 4 + (session_id_len+1) + 28 + 3;
		data = gnutls_malloc ( datalen); 

		data[pos++] = GNUTLS_VERSION_MAJOR;
		data[pos++] = GNUTLS_VERSION_MINOR;		
#ifdef WORDS_BIGENDIAN
		cur_time = time(NULL);
#else
		cur_time = byteswap32(time(NULL));
#endif
		memmove( state->security_parameters.client_random, &cur_time, 4);
		memmove( &state->security_parameters.client_random[4], rand, 28);

		memmove( &data[pos], &cur_time, 4);
		pos += 4;
		memmove( &data[pos], rand, 28);
		pos+=28;

		memmove( &data[pos++], &session_id_len, 1);

		if (session_id_len>0) {
			memmove( &data[pos], SessionID, session_id_len);
		}
		pos+=session_id_len;

		x = _gnutls_supported_ciphersuites( &cipher_suites);

#ifdef WORDS_BIGENDIAN
		memmove( &data[pos], &x, sizeof(uint16));
#else
		x=byteswap16(x);
		memmove( &data[pos], &x, sizeof(uint16));
		x=byteswap16(x);
#endif
		pos+=2;

		datalen += 2*x;
		data = gnutls_realloc( data, datalen);

		for (i=0;i<x;i++) {
			memmove( &data[pos], &cipher_suites[i].CipherSuite, 2);
			pos+=2;
		}

		z = _gnutls_supported_compression_methods( &compression_methods);
		memmove( &data[pos++], &z, sizeof(uint8));
		datalen += z;
		data = gnutls_realloc( data, datalen);

		for (i=0;i<z;i++) {
			memmove( &data[pos], &compression_methods[i], 1);
			pos++;
		}

		gcry_free(rand);
		gnutls_free(cipher_suites);
		gnutls_free(compression_methods);

		ret = _gnutls_send_handshake( cd, state, data, datalen, GNUTLS_CLIENT_HELLO);
		gnutls_free(data);

	
	} else { /* SERVER */
		datalen = 2 + sizeof(uint32) + session_id_len+1 + 28;
		data = gnutls_malloc ( datalen);
	
		data[pos++] = GNUTLS_VERSION_MAJOR;
		data[pos++] = GNUTLS_VERSION_MINOR;
#ifdef WORDS_BIGENDIAN
		cur_time = time(NULL);
#else
		cur_time = byteswap32(time(NULL));
#endif
		memmove( state->security_parameters.server_random, &cur_time, 4);
		memmove( &state->security_parameters.server_random[4], rand, 28);

		memmove( &data[pos], &cur_time, sizeof(uint32));
		pos += sizeof(uint32);
		memmove( &data[pos], rand, 28);
		pos+=28;

		memmove( &data[pos++], &session_id_len, sizeof(uint8));		
		if (session_id_len>0) {
			memmove( &data[pos], SessionID, session_id_len);
		}
		pos+=session_id_len;
		
		datalen += 2;
		data = gnutls_realloc( data, datalen);
		memmove( &data[pos], &state->gnutls_internals.current_cipher_suite.CipherSuite, 2);
		pos+=2;

		datalen += 1;
		data = gnutls_realloc( data, datalen);
		memmove( &data[pos++], &state->gnutls_internals.compression_method, 1);

		gcry_free(rand);
		ret = _gnutls_send_handshake( cd, state, data, datalen, GNUTLS_SERVER_HELLO);
		gnutls_free(data);

	}

	return ret;
}


/* RECEIVE A HELLO MESSAGE. This should be called from gnutls_recv_handshake only if a
 * hello message is expected. It uses the gnutls_internals.current_cipher_suite
 * and gnutls_internals.compression_method.
 */
int _gnutls_recv_hello(int cd, GNUTLS_STATE state, char* data, int datalen, opaque** SessionID, int SessionIDnum) {
	uint8 session_id_len=0, z;
	uint32 cur_time;
	int pos=0;
	GNUTLS_CipherSuite cipher_suite, *cipher_suites;
	CompressionMethod compression_method, *compression_methods;
	int i, ret;
	uint16 x, sizeOfSuites;
	
	if (state->security_parameters.entity == GNUTLS_CLIENT) {
		if (datalen < 38) return GNUTLS_E_UNSUPPORTED_VERSION_PACKET;

		if (data[pos++] != GNUTLS_VERSION_MAJOR) 
			return GNUTLS_E_UNSUPPORTED_VERSION_PACKET;

		if (data[pos++] != GNUTLS_VERSION_MINOR)
			return GNUTLS_E_UNSUPPORTED_VERSION_PACKET;

		memmove( state->security_parameters.server_random, &data[pos], 32);
		pos+=32;
		
		memmove( &session_id_len, &data[pos++], 1);

		if (datalen < 38+session_id_len) return GNUTLS_E_UNSUPPORTED_VERSION_PACKET;
		pos+=session_id_len;
		
		/* We should resume an old connection here. This is not
		 * implemented yet.
		 */

		memmove( &cipher_suite.CipherSuite, &data[pos], 2);
		pos+=2;

		z=1;
		x = _gnutls_supported_ciphersuites( &cipher_suites);
		for (i=0;i<x;i++) {
			if ( memcmp( &cipher_suites[i], cipher_suite.CipherSuite, 2) == 0) {
				z=0;

			}
		}
		if (z!=0) return GNUTLS_E_UNKNOWN_CIPHER_TYPE;
		memmove( state->gnutls_internals.current_cipher_suite.CipherSuite, cipher_suite.CipherSuite, 2);
		
		z=1;
		memmove( &compression_method, &data[pos++], 1);
		z = _gnutls_supported_compression_methods( &compression_methods);
		for (i=0;i<z;i++) {
			if (memcmp( &compression_methods[i], &compression_method, 1)==0) {
				z=0;

			}
		}
		if (z!=0) return GNUTLS_E_UNKNOWN_COMPRESSION_ALGORITHM;
		memmove( &state->gnutls_internals.compression_method, &compression_method, 1);


		gnutls_free(cipher_suites);
		gnutls_free(compression_methods);

	} else { /* Server side reading a client hello */
		if (datalen < 35) return GNUTLS_E_UNSUPPORTED_VERSION_PACKET;

		if (data[pos++] != GNUTLS_VERSION_MAJOR)
			return GNUTLS_E_UNSUPPORTED_VERSION_PACKET;

		if (data[pos++] != GNUTLS_VERSION_MINOR)
			return GNUTLS_E_UNSUPPORTED_VERSION_PACKET;

		memmove( state->security_parameters.client_random, &data[pos], 32);
		pos+=32;
		
		memmove( &session_id_len, &data[pos++], 1);
		pos+=session_id_len;
		
		/* We should resume an old connection here. This is not
		 * implemented yet.
		 */


		/* Select a ciphersuite */
		memmove( &sizeOfSuites, &data[pos], 2);
		pos+=2;
#ifndef WORDS_BIGENDIAN
		sizeOfSuites=byteswap16(sizeOfSuites);
#endif
		SelectSuite( state->gnutls_internals.current_cipher_suite.CipherSuite, &data[pos], sizeOfSuites); 
		pos+=sizeOfSuites;
		
		memmove( &z, &data[pos++], 1);
		SelectCompMethod( &state->gnutls_internals.compression_method, &data[pos], z);
		
	}

	return ret;
}


int gnutls_handshake(int cd, GNUTLS_STATE state) {
int ret;

	if (state->security_parameters.entity == GNUTLS_CLIENT) {
		ret = _gnutls_send_hello( cd, state, NULL, 0);
		if (ret<0) {
			ERR("send hello", ret);
			return ret;
		}
		/* receive the server handshake */
		ret = gnutls_recv_int( cd, state, GNUTLS_HANDSHAKE, NULL, 0);
		if (ret<0) {
			ERR("recv hello", ret);
			return ret;
		}
		ret = gnutls_recv_int( cd, state, GNUTLS_CHANGE_CIPHER_SPEC, NULL, 0);
		if (ret<0) {
			ERR("recv ChangeCipherSpec", ret);
			return ret;
		}
		ret = _gnutls_send_change_cipher_spec( cd, state);
		if (ret<0) {
			ERR("send ChangeCipherSpec", ret);
			return ret;
		}

	} else { /* SERVER */
		
		ret = gnutls_recv_int( cd, state, GNUTLS_HANDSHAKE, NULL, 0);
		if (ret<0) {
			ERR("recv hello", ret);
			return ret;
		}
		ret = _gnutls_send_hello( cd, state, NULL, 0);
		if (ret<0) {
			ERR("send hello", ret);
			return ret;
		}
		ret = _gnutls_send_change_cipher_spec( cd, state);
		if (ret<0) {
			ERR("send ChangeCipherSpec", ret);
			return ret;
		}
		ret = gnutls_recv_int( cd, state, GNUTLS_CHANGE_CIPHER_SPEC, NULL, 0);
		if (ret<0) {
			ERR("recv ChangeCipherSpec", ret);
			return ret;
		}
	}

	return ret;

}