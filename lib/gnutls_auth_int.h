int gnutls_clear_creds( gnutls_session session);
int gnutls_credentials_set( gnutls_session session, gnutls_credentials_type type, void* cred);
const void *_gnutls_get_cred( GNUTLS_KEY key, gnutls_credentials_type kx, int* err);
const void *_gnutls_get_kx_cred( GNUTLS_KEY key, gnutls_kx_algorithm algo, int *err);
int _gnutls_generate_key(GNUTLS_KEY key);
gnutls_credentials_type gnutls_auth_get_type( gnutls_session session);
void* _gnutls_get_auth_info( gnutls_session session);
int _gnutls_auth_info_set( gnutls_session session, gnutls_credentials_type type, int size, int allow_change);
