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

/**
 * @brief Create esma machine template.
 * @param [in] name	esma template name.
 * @return esma_template - if esma machine template successfuly created; NULL - otherwise.
 */
struct esma_template *esma_template_new(char *name);

/**
 * @brief Initialize esma machine template.
 * @param [out] template	Pointer to esma machine template.
 * @param [in] name		esma template name.
 * @return 0 - if esma template successfuly initialized; 1 - otherwise.
 */
int esma_template_init(struct esma_template *template, char *name);

/**
 * @brief Release esma machine template memory.
 * @param [out] template	Pointer to esma machine template.
 */
void esma_template_free(struct esma_template *template);

/**
 * @brief Set esma machine template by configuration (*.esma) file.
 * @param [out] template	Pointer to esma machine template.
 * @param [in] path		Path to esma configuration path.
 * @return 0 - if esma template successfuly seted; 1 - otherwise.
 */
int esma_template_set_by_path(struct esma_template *template, char *path);

/**
 * @brief Set esma machine template by esma_dbuf.
 * @param [out] template	Pointer to esma machine template.
 * @param [in] dbuf		Pointer to dbuf.
 * @return 0 - if esma template successfuly seted; 1 - otherwise.
 */
int esma_template_set_by_dbuf(struct esma_template *tmpl, struct esma_dbuf *dbuf);

/**
 * @brief Print esma template.
 * @param [in] template	Pointer to esma machine template.
 * @details Use this function for debug.
 */
void esma_template_print(struct esma_template *tmpl);
#endif
