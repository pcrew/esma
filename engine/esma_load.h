/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_LOAD_H
#define ESMA_LOAD_H

/**
 * @brief Load esma machine.
 * @param [out] esma	Pointer to the engine.
 * @param [in] template	Pointer to the reactor name.
 * @return 0 - if esma machine successfuly loaded; 1 - otherwise.
 */
int esma_load(struct esma *esma, struct esma_template *template);

#endif
