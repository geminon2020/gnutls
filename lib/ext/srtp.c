/*
 * Copyright (C) 2012 Martin Storsjo
 * 
 * Author: Martin Storsjo
 *
 * This file is part of GnuTLS.
 *
 * The GnuTLS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#include "gnutls_int.h"
#include "gnutls_auth.h"
#include "gnutls_errors.h"
#include "gnutls_num.h"
#include <ext/srtp.h>

static int _gnutls_srtp_recv_params (gnutls_session_t session,
                                     const uint8_t * data,
                                     size_t data_size);
static int _gnutls_srtp_send_params (gnutls_session_t session,
                                     gnutls_buffer_st* extdata);

static int _gnutls_srtp_unpack (gnutls_buffer_st * ps,
                                extension_priv_data_t * _priv);
static int _gnutls_srtp_pack (extension_priv_data_t _priv,
                              gnutls_buffer_st * ps);
static void _gnutls_srtp_deinit_data (extension_priv_data_t priv);


extension_entry_st ext_mod_srtp = {
  .name = "SRTP",
  .type = GNUTLS_EXTENSION_SRTP,
  .parse_type = GNUTLS_EXT_APPLICATION,

  .recv_func = _gnutls_srtp_recv_params,
  .send_func = _gnutls_srtp_send_params,
  .pack_func = _gnutls_srtp_pack,
  .unpack_func = _gnutls_srtp_unpack,
  .deinit_func = _gnutls_srtp_deinit_data,
};

typedef struct
{
  const char *name;
  gnutls_srtp_profile_t id;
} srtp_profile_st;

static const srtp_profile_st profile_names[] = {
  {
    "SRTP_AES128_CM_SHA1_80",
    GNUTLS_SRTP_AES128_CM_SHA1_80,
  },
  {
    "SRTP_AES128_CM_SHA1_32",
    GNUTLS_SRTP_AES128_CM_SHA1_32,
  },
  {
    "SRTP_NULL_SHA1_80",
    GNUTLS_SRTP_NULL_SHA1_80,
  },
  {
    "SRTP_NULL_SHA1_32",
    GNUTLS_SRTP_NULL_SHA1_32,
  },
  {
    NULL,
    0
  }
};

static gnutls_srtp_profile_t find_profile (const char *str, const char *end)
{
  const srtp_profile_st *prof = profile_names;
  unsigned int len;
  if (end != NULL)
    {
      len = end - str;
    }
  else
    {
      len = strlen (str);
    }

  while (prof->name != NULL)
    {
      if (strlen (prof->name) == len && !strncmp (str, prof->name, len))
        {
          return prof->id;
        }
      prof++;
    }
  return 0;
}

/**
 * gnutls_srtp_get_profile_id
 * @name: The name of the profile to look up
 * @profile: Will hold the profile id
 *
 * This function allows you to look up a profile based on a string.
 *
 * Returns: On success, %GNUTLS_E_SUCCESS (0) is returned,
 *   otherwise a negative error code is returned.
 *
 * Since 3.1.4
 **/
int gnutls_srtp_get_profile_id (const char *name,
                                gnutls_srtp_profile_t *profile)
{
  *profile = find_profile (name, NULL);
  if (*profile == 0)
    {
      return GNUTLS_E_ILLEGAL_PARAMETER;
    }
  return 0;
}

/**
 * gnutls_srtp_get_profile_name
 * @profile: The profile to look up a string for
 *
 * This function allows you to get the corresponding name for a
 * SRTP protection profile.
 *
 * Returns: On success, the name of a SRTP profile as a string,
 *   otherwise NULL.
 *
 * Since 3.1.4
 **/
const char *gnutls_srtp_get_profile_name (gnutls_srtp_profile_t profile)
{
  const srtp_profile_st *p = profile_names;
  while (p->name != NULL)
    {
      if (p->id == profile)
        return p->name;
      p++;
    }
  return NULL;
}

static int
_gnutls_srtp_recv_params (gnutls_session_t session,
                          const uint8_t *data, size_t _data_size)
{
  unsigned int i, j;
  int ret;
  const uint8_t *p = data;
  int len;
  ssize_t data_size = _data_size;
  srtp_ext_st *priv;
  extension_priv_data_t epriv;
  uint16_t profiles[MAX_SRTP_PROFILES];
  unsigned int profiles_size = 0;

  ret =
    _gnutls_ext_get_session_data (session, GNUTLS_EXTENSION_SRTP,
                                  &epriv);
  if (ret < 0)
    return 0;

  priv = epriv.ptr;

  DECR_LENGTH_RET (data_size, 2, 0);
  len = _gnutls_read_uint16 (p);
  p += 2;

  while (len > 0)
    {
      DECR_LENGTH_RET (data_size, 2, 0);
      if (profiles_size < MAX_SRTP_PROFILES)
        profiles_size++;
      profiles[profiles_size - 1] = _gnutls_read_uint16 (p);
      p += 2;
      len -= 2;
    }

  for (i = 0; i < priv->profiles_size && priv->selected_profile == 0; i++)
    {
      for (j = 0; j < profiles_size; j++)
        {
          if (priv->profiles[i] == profiles[j])
            {
              priv->selected_profile = profiles[j];
              break;
            }
        }
    }

  return 0;
}

static int
_gnutls_srtp_send_params (gnutls_session_t session,
                          gnutls_buffer_st* extdata)
{
  unsigned i;
  int total_size = 0, ret;
  srtp_ext_st *priv;
  extension_priv_data_t epriv;

  ret =
    _gnutls_ext_get_session_data (session, GNUTLS_EXTENSION_SRTP,
                                  &epriv);
  if (ret < 0)
    return 0;

  priv = epriv.ptr;

  if (priv->profiles_size == 0)
    return 0;

  if (session->security_parameters.entity == GNUTLS_SERVER)
    {
      /* Don't send anything if no matching profile was found */
      if (priv->selected_profile == 0)
        return 0;

      ret = _gnutls_buffer_append_prefix(extdata, 16, 2);
      if (ret < 0)
        return gnutls_assert_val(ret);
      ret = _gnutls_buffer_append_prefix(extdata, 16, priv->selected_profile);
      if (ret < 0)
        return gnutls_assert_val(ret);
      total_size = 4;
    }
  else
    {
      ret = _gnutls_buffer_append_prefix(extdata, 16, 2 * priv->profiles_size);
      if (ret < 0)
        return gnutls_assert_val(ret);

      for (i = 0; i < priv->profiles_size; i++)
        {
          ret = _gnutls_buffer_append_prefix(extdata, 16, priv->profiles[i]);
          if (ret < 0)
            return gnutls_assert_val(ret);
        }
      total_size = 2 + 2 * priv->profiles_size;
    }

  /* use_mki, not supported yet */
  ret = _gnutls_buffer_append_prefix(extdata, 8, 0);
  if (ret < 0)
    return gnutls_assert_val(ret);

  return total_size + 1;
}

/**
 * gnutls_srtp_get_selected_profile:
 * @session: is a #gnutls_session_t structure.
 * @profile: will hold the profile
 *
 * This function allows you to get the negotiated SRTP profile.
 *
 * Returns: On success, %GNUTLS_E_SUCCESS (0) is returned,
 *   otherwise a negative error code is returned.
 *
 * Since 3.1.4
 **/
int
gnutls_srtp_get_selected_profile (gnutls_session_t session,
                                  gnutls_srtp_profile_t *profile)
{
  srtp_ext_st *priv;
  int ret;
  extension_priv_data_t epriv;

  ret =
    _gnutls_ext_get_session_data (session, GNUTLS_EXTENSION_SRTP,
                                  &epriv);
  if (ret < 0)
    {
      gnutls_assert ();
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  priv = epriv.ptr;

  if (priv->selected_profile == 0)
    {
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  *profile = priv->selected_profile;

  return 0;
}

/**
 * gnutls_srtp_set_profile:
 * @session: is a #gnutls_session_t structure.
 * @profile: is the profile id to add.
 *
 * This function is to be used by both clients and servers, to declare
 * what SRTP profiles they support, to negotiate with the peer.
 *
 * Returns: On success, %GNUTLS_E_SUCCESS (0) is returned,
 *   otherwise a negative error code is returned.
 *
 * Since 3.1.4
 **/
int
gnutls_srtp_set_profile (gnutls_session_t session,
                         gnutls_srtp_profile_t profile)
{
  int ret;
  srtp_ext_st *priv;
  extension_priv_data_t epriv;

  ret =
    _gnutls_ext_get_session_data (session, GNUTLS_EXTENSION_SRTP,
                                  &epriv);
  if (ret < 0)
    {
      priv = gnutls_calloc (1, sizeof (*priv));
      if (priv == NULL)
        {
          gnutls_assert ();
          return GNUTLS_E_MEMORY_ERROR;
        }
      epriv.ptr = priv;
      _gnutls_ext_set_session_data (session, GNUTLS_EXTENSION_SRTP,
                                    epriv);
    }
  else
    priv = epriv.ptr;

  if (priv->profiles_size < MAX_SRTP_PROFILES)
    priv->profiles_size++;
  priv->profiles[priv->profiles_size - 1] = profile;

  return 0;
}

/**
 * gnutls_srtp_set_profile_direct:
 * @session: is a #gnutls_session_t structure.
 * @profiles: is a string that contains the supported SRTP profiles,
 *   separated by colons.
 * @err_pos: In case of an error this will have the position in the string the error occured, may be NULL.
 *
 * This function is to be used by both clients and servers, to declare
 * what SRTP profiles they support, to negotiate with the peer.
 *
 * Returns: On syntax error %GNUTLS_E_INVALID_REQUEST is returned,
 * %GNUTLS_E_SUCCESS on success, or an error code.
 *
 * Since 3.1.4
 **/
int
gnutls_srtp_set_profile_direct (gnutls_session_t session,
                                const char *profiles, const char **err_pos)
{
  int ret;
  srtp_ext_st *priv;
  extension_priv_data_t epriv;
  int set = 0;
  const char *col;
  gnutls_srtp_profile_t id;

  ret =
    _gnutls_ext_get_session_data (session, GNUTLS_EXTENSION_SRTP,
                                  &epriv);
  if (ret < 0)
    {
      set = 1;
      priv = gnutls_calloc (1, sizeof (*priv));
      if (priv == NULL)
        {
          if (err_pos != NULL)
            *err_pos = profiles;
          gnutls_assert ();
          return GNUTLS_E_MEMORY_ERROR;
        }
      epriv.ptr = priv;
    }
  else
    priv = epriv.ptr;

  do
    {
      col = strchr (profiles, ':');
      id = find_profile (profiles, col);
      if (id == 0)
        {
          if (set != 0)
              gnutls_free (priv);
          if (err_pos != NULL)
            *err_pos = profiles;
          return GNUTLS_E_INVALID_REQUEST;
        }

      if (priv->profiles_size < MAX_SRTP_PROFILES)
        {
          priv->profiles_size++;
        }
      priv->profiles[priv->profiles_size - 1] = id;
      profiles = col + 1;
    } while (col != NULL);

  if (set != 0)
    _gnutls_ext_set_session_data (session, GNUTLS_EXTENSION_SRTP,
                                  epriv);

  return 0;
}

static void
_gnutls_srtp_deinit_data (extension_priv_data_t priv)
{
  gnutls_free (priv.ptr);
}

static int
_gnutls_srtp_pack (extension_priv_data_t epriv, gnutls_buffer_st * ps)
{
  srtp_ext_st *priv = epriv.ptr;
  unsigned int i;
  int ret;

  BUFFER_APPEND_NUM (ps, priv->profiles_size);
  for (i = 0; i < priv->profiles_size; i++)
    {
      BUFFER_APPEND_NUM (ps, priv->profiles[i]);
    }
  BUFFER_APPEND_NUM (ps, priv->selected_profile);
  return 0;
}

static int
_gnutls_srtp_unpack (gnutls_buffer_st * ps,
                     extension_priv_data_t * _priv)
{
  srtp_ext_st *priv;
  unsigned int i;
  int ret;
  extension_priv_data_t epriv;

  priv = gnutls_calloc (1, sizeof (*priv));
  if (priv == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_MEMORY_ERROR;
    }

  BUFFER_POP_NUM (ps, priv->profiles_size);
  for (i = 0; i < priv->profiles_size; i++)
    {
      BUFFER_POP_NUM (ps, priv->profiles[i]);
    }
  BUFFER_POP_NUM (ps, priv->selected_profile);

  epriv.ptr = priv;
  *_priv = epriv;

  return 0;

error:
  gnutls_free (priv);
  return ret;
}
