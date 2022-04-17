/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_TEMPLATE_H
#define ESMA_TEMPLATE_H

#include <stdlib.h>
#include <string.h>

#include "esma.h"
#include "core/esma_dbuf.h"
#include "core/esma_array.h"
#include "common/numeric_types.h"

struct esma_template *esma_template_new(char *name);
   int esma_template_init(struct esma_template *tmpl, char *name);
  void esma_template_free(struct esma_template *tmpl);
   int esma_template_set_by_path(struct esma_template *tmpl, char *path);
   int esma_template_set_by_dbuf(struct esma_template *tmpl, struct esma_dbuf *dbuf);

  void esma_template_print(struct esma_template *tmpl);	/* for debug */
#endif
