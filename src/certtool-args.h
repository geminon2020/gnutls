/*   -*- buffer-read-only: t -*- vi: set ro:
 *  
 *  DO NOT EDIT THIS FILE   (certtool-args.h)
 *  
 *  It has been AutoGen-ed  November 21, 2012 at 09:00:25 PM by AutoGen 5.16
 *  From the definitions    certtool-args.def
 *  and the template file   options
 *
 * Generated from AutoOpts 36:4:11 templates.
 *
 *  AutoOpts is a copyrighted work.  This header file is not encumbered
 *  by AutoOpts licensing, but is provided under the licensing terms chosen
 *  by the certtool author or copyright holder.  AutoOpts is
 *  licensed under the terms of the LGPL.  The redistributable library
 *  (``libopts'') is licensed under the terms of either the LGPL or, at the
 *  users discretion, the BSD license.  See the AutoOpts and/or libopts sources
 *  for details.
 *
 * The certtool program is copyrighted and licensed
 * under the following terms:
 *
 *  Copyright (C) 2000-2012 Free Software Foundation, all rights reserved.
 *  This is free software. It is licensed for use, modification and
 *  redistribution under the terms of the
 *  GNU General Public License, version 3 or later
 *      <http://gnu.org/licenses/gpl.html>
 *
 *  certtool is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  certtool is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 *  This file contains the programmatic interface to the Automated
 *  Options generated for the certtool program.
 *  These macros are documented in the AutoGen info file in the
 *  "AutoOpts" chapter.  Please refer to that doc for usage help.
 */
#ifndef AUTOOPTS_CERTTOOL_ARGS_H_GUARD
#define AUTOOPTS_CERTTOOL_ARGS_H_GUARD 1
#include "config.h"
#include <autoopts/options.h>

/*
 *  Ensure that the library used for compiling this generated header is at
 *  least as new as the version current when the header template was released
 *  (not counting patch version increments).  Also ensure that the oldest
 *  tolerable version is at least as old as what was current when the header
 *  template was released.
 */
#define AO_TEMPLATE_VERSION 147460
#if (AO_TEMPLATE_VERSION < OPTIONS_MINIMUM_VERSION) \
 || (AO_TEMPLATE_VERSION > OPTIONS_STRUCT_VERSION)
# error option template version mismatches autoopts/options.h header
  Choke Me.
#endif

/*
 *  Enumeration of each option:
 */
typedef enum {
    INDEX_OPT_DEBUG                 =  0,
    INDEX_OPT_VERBOSE               =  1,
    INDEX_OPT_INFILE                =  2,
    INDEX_OPT_OUTFILE               =  3,
    INDEX_OPT_GENERATE_SELF_SIGNED  =  4,
    INDEX_OPT_GENERATE_CERTIFICATE  =  5,
    INDEX_OPT_GENERATE_PROXY        =  6,
    INDEX_OPT_GENERATE_CRL          =  7,
    INDEX_OPT_UPDATE_CERTIFICATE    =  8,
    INDEX_OPT_GENERATE_PRIVKEY      =  9,
    INDEX_OPT_GENERATE_REQUEST      = 10,
    INDEX_OPT_VERIFY_CHAIN          = 11,
    INDEX_OPT_VERIFY                = 12,
    INDEX_OPT_VERIFY_CRL            = 13,
    INDEX_OPT_GENERATE_DH_PARAMS    = 14,
    INDEX_OPT_GET_DH_PARAMS         = 15,
    INDEX_OPT_DH_INFO               = 16,
    INDEX_OPT_LOAD_PRIVKEY          = 17,
    INDEX_OPT_LOAD_PUBKEY           = 18,
    INDEX_OPT_LOAD_REQUEST          = 19,
    INDEX_OPT_LOAD_CERTIFICATE      = 20,
    INDEX_OPT_LOAD_CA_PRIVKEY       = 21,
    INDEX_OPT_LOAD_CA_CERTIFICATE   = 22,
    INDEX_OPT_PASSWORD              = 23,
    INDEX_OPT_HEX_NUMBERS           = 24,
    INDEX_OPT_NULL_PASSWORD         = 25,
    INDEX_OPT_CERTIFICATE_INFO      = 26,
    INDEX_OPT_CERTIFICATE_PUBKEY    = 27,
    INDEX_OPT_PGP_CERTIFICATE_INFO  = 28,
    INDEX_OPT_PGP_RING_INFO         = 29,
    INDEX_OPT_CRL_INFO              = 30,
    INDEX_OPT_CRQ_INFO              = 31,
    INDEX_OPT_NO_CRQ_EXTENSIONS     = 32,
    INDEX_OPT_P12_INFO              = 33,
    INDEX_OPT_P7_INFO               = 34,
    INDEX_OPT_SMIME_TO_P7           = 35,
    INDEX_OPT_KEY_INFO              = 36,
    INDEX_OPT_PGP_KEY_INFO          = 37,
    INDEX_OPT_PUBKEY_INFO           = 38,
    INDEX_OPT_V1                    = 39,
    INDEX_OPT_TO_P12                = 40,
    INDEX_OPT_TO_P8                 = 41,
    INDEX_OPT_PKCS8                 = 42,
    INDEX_OPT_RSA                   = 43,
    INDEX_OPT_DSA                   = 44,
    INDEX_OPT_ECC                   = 45,
    INDEX_OPT_ECDSA                 = 46,
    INDEX_OPT_HASH                  = 47,
    INDEX_OPT_INDER                 = 48,
    INDEX_OPT_INRAW                 = 49,
    INDEX_OPT_OUTDER                = 50,
    INDEX_OPT_OUTRAW                = 51,
    INDEX_OPT_BITS                  = 52,
    INDEX_OPT_SEC_PARAM             = 53,
    INDEX_OPT_DISABLE_QUICK_RANDOM  = 54,
    INDEX_OPT_TEMPLATE              = 55,
    INDEX_OPT_PKCS_CIPHER           = 56,
    INDEX_OPT_VERSION               = 57,
    INDEX_OPT_HELP                  = 58,
    INDEX_OPT_MORE_HELP             = 59
} teOptIndex;

#define OPTION_CT    60
#define CERTTOOL_VERSION       "@VERSION@"
#define CERTTOOL_FULL_VERSION  "certtool @VERSION@"

/*
 *  Interface defines for all options.  Replace "n" with the UPPER_CASED
 *  option name (as in the teOptIndex enumeration above).
 *  e.g. HAVE_OPT(DEBUG)
 */
#define         DESC(n) (certtoolOptions.pOptDesc[INDEX_OPT_## n])
#define     HAVE_OPT(n) (! UNUSED_OPT(& DESC(n)))
#define      OPT_ARG(n) (DESC(n).optArg.argString)
#define    STATE_OPT(n) (DESC(n).fOptState & OPTST_SET_MASK)
#define    COUNT_OPT(n) (DESC(n).optOccCt)
#define    ISSEL_OPT(n) (SELECTED_OPT(&DESC(n)))
#define ISUNUSED_OPT(n) (UNUSED_OPT(& DESC(n)))
#define  ENABLED_OPT(n) (! DISABLED_OPT(& DESC(n)))
#define  STACKCT_OPT(n) (((tArgList*)(DESC(n).optCookie))->useCt)
#define STACKLST_OPT(n) (((tArgList*)(DESC(n).optCookie))->apzArgs)
#define    CLEAR_OPT(n) STMTS( \
                DESC(n).fOptState &= OPTST_PERSISTENT_MASK;   \
                if ((DESC(n).fOptState & OPTST_INITENABLED) == 0) \
                    DESC(n).fOptState |= OPTST_DISABLED; \
                DESC(n).optCookie = NULL )

/* * * * * *
 *
 *  Enumeration of certtool exit codes
 */
typedef enum {
    CERTTOOL_EXIT_SUCCESS = 0,
    CERTTOOL_EXIT_FAILURE = 1,
    CERTTOOL_EXIT_LIBOPTS_FAILURE = 70
} certtool_exit_code_t;
/* * * * * *
 *
 *  Interface defines for specific options.
 */
#define VALUE_OPT_DEBUG          'd'

#define OPT_VALUE_DEBUG          (DESC(DEBUG).optArg.argInt)
#define VALUE_OPT_VERBOSE        'V'
#define VALUE_OPT_INFILE         2
#define VALUE_OPT_OUTFILE        3
#define VALUE_OPT_GENERATE_SELF_SIGNED 's'
#define VALUE_OPT_GENERATE_CERTIFICATE 'c'
#define VALUE_OPT_GENERATE_PROXY 6
#define VALUE_OPT_GENERATE_CRL   7
#define VALUE_OPT_UPDATE_CERTIFICATE 'u'
#define VALUE_OPT_GENERATE_PRIVKEY 'p'
#define VALUE_OPT_GENERATE_REQUEST 'q'
#define VALUE_OPT_VERIFY_CHAIN   'e'
#define VALUE_OPT_VERIFY         12
#define VALUE_OPT_VERIFY_CRL     13
#define VALUE_OPT_GENERATE_DH_PARAMS 14
#define VALUE_OPT_GET_DH_PARAMS  15
#define VALUE_OPT_DH_INFO        16
#define VALUE_OPT_LOAD_PRIVKEY   17
#define VALUE_OPT_LOAD_PUBKEY    18
#define VALUE_OPT_LOAD_REQUEST   19
#define VALUE_OPT_LOAD_CERTIFICATE 20
#define VALUE_OPT_LOAD_CA_PRIVKEY 21
#define VALUE_OPT_LOAD_CA_CERTIFICATE 22
#define VALUE_OPT_PASSWORD       23
#define VALUE_OPT_HEX_NUMBERS    24
#define VALUE_OPT_NULL_PASSWORD  25
#define VALUE_OPT_CERTIFICATE_INFO 'i'
#define VALUE_OPT_CERTIFICATE_PUBKEY 27
#define VALUE_OPT_PGP_CERTIFICATE_INFO 28
#define VALUE_OPT_PGP_RING_INFO  29
#define VALUE_OPT_CRL_INFO       'l'
#define VALUE_OPT_CRQ_INFO       31
#define VALUE_OPT_NO_CRQ_EXTENSIONS 32
#define VALUE_OPT_P12_INFO       129
#define VALUE_OPT_P7_INFO        130
#define VALUE_OPT_SMIME_TO_P7    131
#define VALUE_OPT_KEY_INFO       'k'
#define VALUE_OPT_PGP_KEY_INFO   133
#define VALUE_OPT_PUBKEY_INFO    134
#define VALUE_OPT_V1             135
#define VALUE_OPT_TO_P12         136
#define VALUE_OPT_TO_P8          137
#define VALUE_OPT_PKCS8          '8'
#define VALUE_OPT_RSA            139
#define VALUE_OPT_DSA            140
#define VALUE_OPT_ECC            141
#define VALUE_OPT_ECDSA          142
#define VALUE_OPT_HASH           143
#define VALUE_OPT_INDER          144
#define VALUE_OPT_INRAW          145
#define VALUE_OPT_OUTDER         146
#define VALUE_OPT_OUTRAW         147
#define VALUE_OPT_BITS           148

#define OPT_VALUE_BITS           (DESC(BITS).optArg.argInt)
#define VALUE_OPT_SEC_PARAM      149
#define VALUE_OPT_DISABLE_QUICK_RANDOM 150
#define VALUE_OPT_TEMPLATE       151
#define VALUE_OPT_PKCS_CIPHER    152
#define VALUE_OPT_HELP          'h'
#define VALUE_OPT_MORE_HELP     '!'
#define VALUE_OPT_VERSION       'v'
/*
 *  Interface defines not associated with particular options
 */
#define ERRSKIP_OPTERR  STMTS(certtoolOptions.fOptSet &= ~OPTPROC_ERRSTOP)
#define ERRSTOP_OPTERR  STMTS(certtoolOptions.fOptSet |= OPTPROC_ERRSTOP)
#define RESTART_OPT(n)  STMTS( \
                certtoolOptions.curOptIdx = (n); \
                certtoolOptions.pzCurOpt  = NULL)
#define START_OPT       RESTART_OPT(1)
#define USAGE(c)        (*certtoolOptions.pUsageProc)(&certtoolOptions, c)
/* extracted from opthead.tlib near line 484 */

#ifdef  __cplusplus
extern "C" {
#endif
/*
 *  global exported definitions
 */
#include <gettext.h>


/* * * * * *
 *
 *  Declare the certtool option descriptor.
 */
extern tOptions certtoolOptions;

#if defined(ENABLE_NLS)
# ifndef _
#   include <stdio.h>
static inline char* aoGetsText(char const* pz) {
    if (pz == NULL) return NULL;
    return (char*)gettext(pz);
}
#   define _(s)  aoGetsText(s)
# endif /* _() */

# define OPT_NO_XLAT_CFG_NAMES  STMTS(certtoolOptions.fOptSet |= \
                                    OPTPROC_NXLAT_OPT_CFG;)
# define OPT_NO_XLAT_OPT_NAMES  STMTS(certtoolOptions.fOptSet |= \
                                    OPTPROC_NXLAT_OPT|OPTPROC_NXLAT_OPT_CFG;)

# define OPT_XLAT_CFG_NAMES     STMTS(certtoolOptions.fOptSet &= \
                                  ~(OPTPROC_NXLAT_OPT|OPTPROC_NXLAT_OPT_CFG);)
# define OPT_XLAT_OPT_NAMES     STMTS(certtoolOptions.fOptSet &= \
                                  ~OPTPROC_NXLAT_OPT;)

#else   /* ENABLE_NLS */
# define OPT_NO_XLAT_CFG_NAMES
# define OPT_NO_XLAT_OPT_NAMES

# define OPT_XLAT_CFG_NAMES
# define OPT_XLAT_OPT_NAMES

# ifndef _
#   define _(_s)  _s
# endif
#endif  /* ENABLE_NLS */

#ifdef  __cplusplus
}
#endif
#endif /* AUTOOPTS_CERTTOOL_ARGS_H_GUARD */
/* certtool-args.h ends here */
