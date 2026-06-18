/*
* Copyright (c) 2002, 2014, Oracle and/or its affiliates. All rights reserved.
 *
 * This material is the confidential property of Oracle Corporation
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static const char Sccs_id[] = "@(#)$Id: fm_nai_config.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 04:09:40 nishahan Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <pinlog.h>
#include "ops/naig3_custom_ops.h"
#include "pcm.h"
#include "cm_fm.h"

#define FILE_LOGNAME "fm_nai_config.c"

PIN_EXPORT void * fm_nai_config_func();

struct cm_fm_config fm_nai_config[] = {
{ NAI3_OP_COMMIT_CUSTOMER, "op_nai3_commit_customer" },
{ NAI3_OP_GET_AVAILABLE_RES, "op_nai3_get_available_res" },
{ NAI3_OP_ACT_RATE, "op_nai3_act_rate" },
{ 0,    (char *)0 }
};

void *
fm_nai_config_func()
{
  return ((void *) (fm_nai_config));
}
