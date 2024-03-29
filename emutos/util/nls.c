/*
 * nls.c - Native Language Support
 *
 * Copyright (c) 2001 Emutos Development Team
 *
 * Authors:
 *  LVL   Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#include "config.h"
#include "portab.h"
 
#include "nls.h"
#include "langs.h"
#include "string.h"
#include "i18nconf.h"

#if CONF_WITH_NLS

/* used by nlsasm.S */

const char * const * const *nls_hash;

void gettext_init(void); /* call each time the hash changes */

/* initialisation */

void nls_init(void)
{
  nls_hash = 0;
  gettext_init();
}

/* functions to query the lang database and to set the lang */

void nls_set_lang(const char *s)
{
  int i;

  for(i = 0 ; langs[i] ; i++) {
    if(!strcmp(s, langs[i]->name)) {
      nls_hash = langs[i]->hash;
      gettext_init();
      return ;
    }
  }
}
  
#endif /* CONF_WITH_NLS */
