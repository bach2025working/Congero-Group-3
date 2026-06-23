/*
 * @(#)% %
 *
 * Copyright (c) 2002 - 2006 Oracle. All rights reserved.
 *
 * This material is the confidential property of Oracle Corporation
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid Oracle license or sublicense agreement.
 *
 */

/*
 * Notes:
 * All CREATE functions, like PIN_FLIST_CREATE or PIN_POID_CREATE does memory allocation, and PCM call too, so destroy after use
 * don't destroy our universal return FLIST
 */

#ifndef lint
static const char Sccs_id[] = "@(#)% %";
#endif

/*******************************************************************
 * Contains the NAI3_OP_ACT_RATE operation.
 *******************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <pcm.h>
#include "ops/act.h"
#include "ops/custom_flds.h"
#include <pinlog.h>

#define FILE_LOGNAME "fm_nai_act_rate.c(1)"

#include "ops/naig3_custom_ops.h"
#include "cm_fm.h"
#include "pin_errs.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_nai_act_rate(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static int64
convert_to_epoch(
        const char              *date_str);

static void search_login(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        char                    *login,
        pin_flist_t             **service_flist,
        poid_t                  **service_poid,
        poid_t                  **account_obj,
        pin_errbuf_t            *ebufp);

static void
fm_nai_act_rate(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        poid_t                  *account_obj,
        char                    *date,
        char                    *descr,
        int32                   *model_code,
        pin_decimal_t           *trans_id,
        char                    *prompt_txt,
        int64                   *tokens_in,
        int64                   *tokens_out,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/


/*******************************************************************
 * Main routine for the NAI3_OP_ACT_RATE operation.
 *******************************************************************/
void
op_nai_act_rate(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pcm_context_t           *ctxp = connp->dm_ctx;
        poid_t                  *acct_poidp = NULL;

        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        /***********************************************************
         * Null out results
         ***********************************************************/
        *r_flistpp = NULL;

        /***********************************************************
         * Insanity check.
         ***********************************************************/
        if (opcode != NAI3_OP_ACT_RATE) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_nai_act_rate opcode error", ebufp);

                return;
        }

        /***********************************************************
         * Debut what we got.
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_nai_act_rate input flist", i_flistp);

        /***********************************************************
         * Main rountine for this opcode
         ***********************************************************/

        char err_msg[256] = "Login not found";

        /*
         * PIN_FLD_ACCOUNT_OBJ is not a required fields, but we use it to retrive the database number for the poids
         */ 
        poid_t * a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        char * login = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 0, ebufp);
        // date will hold the actual date time in the format month-date-year and convert to epoch time
        char * date = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME, 0, ebufp);
        char * descr = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 0, ebufp);
        int32 *model_code = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODELTYPE_NAI3, 0, ebufp);
        pin_decimal_t *trans_id = (pin_decimal_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TRANSACTION_ID, 0, ebufp);
        char * prompt_txt = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROMPT_NAI3, 0, ebufp);
        int64 * tokens_in = (int64 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_INPUT_TOKENS_NAI3, 0, ebufp);
        int64 * tokens_out = (int64 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OUTPUT_TOKENS_NAI3, 0, ebufp);

        /*
         * service_flist will hold the PIN_FLD_RESULTS 
         * and service_poid will hold the poid of the service (via TAKE)
         */ 
        poid_t *service_poid = NULL;
        poid_t *account_obj = NULL;
        pin_flist_t * service_flist = NULL;
        search_login(ctxp, flags, a_pdp, login, &service_flist, &service_poid, &account_obj, ebufp);

        /*
         * service_poid is in the form of 0.0.0.1 /service/my_service XXXXX 0
         * we can cast a char * to it and do a strstr command
         * strstr(word, pattern) returns a pointer to the first letter of pattern inside word,
         * granted pattern exists in word, else return NULL
         */
        char * pattern = "service/nextaig3";
        char * is_nextaig3 = NULL;
        char *poid_str = NULL;
        int32 len = 0; // To store the length of the string representation

        if (service_poid) {
            PIN_POID_TO_STR((poid_t *)service_poid, &poid_str, &len, ebufp);
            if (!PIN_ERR_IS_ERR(ebufp)) {
                is_nextaig3 = strstr(poid_str, pattern);
            }
        }

        /*
         * If the user is not registered as /service/nextaig3, or he just dne
         * we return error code 1 with a list of missing fields
         * Note: destroy the input flist since we're returning
         */ 
        if (!is_nextaig3) {
            pin_flist_t *r_flistp = PIN_FLIST_CREATE(ebufp);
            poid_t *err_poid = PIN_POID_CREATE(1, "/error", -1, ebufp);
            PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, err_poid, ebufp);
            PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE,
                              (void *)"2", ebufp);
            PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
                              (void *)err_msg, ebufp);
            *r_flistpp = r_flistp;

            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, err_msg, ebufp);
            /* PIN_FLIST_DESTROY_EX(&i_flistp, ebufp); */
            return;
        }

        /*
         * Calling our act_rate
         */ 
        fm_nai_act_rate(ctxp, flags, a_pdp, account_obj, date, descr, model_code, trans_id, prompt_txt, tokens_in, tokens_out, r_flistpp, ebufp);

        /***********************************************************
         * Error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_FLIST_DESTROY_EX(r_flistpp, ebufp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_nai_act_rate error", ebufp);
        } else {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_nai_act_rate output flist", *r_flistpp);
        }

        /* PIN_FLIST_DESTROY_EX(&i_flistp, ebufp); */
        PIN_FLIST_DESTROY_EX(&service_flist, ebufp);

        return;
}

/*******************************************************************
 * convert_to_epoch:
 *
 * This function takes a string in the format: mm-dd-yyyy
 * It returns the epoch time if success, else -1
 *
 *******************************************************************/
static int64
convert_to_epoch(
        const char              *date_str)
{
        int month, day, year;

        if (sscanf(date_str, "%d-%d-%d", &month, &day, &year) != 3) {
            return -1;
        }

        if (month < 1 || month > 12 || day < 1 || day > 31 || year < 1900) {
            return -1;
        }

            struct tm tm = {0};
            tm.tm_year = year - 1900;
            tm.tm_mon = month - 1;
            tm.tm_mday = day;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            tm.tm_isdst = -1;

            time_t epoch = mktime(&tm);
            if (epoch == -1) {
                return -1;
            }

            return (int64) epoch;
}

/*******************************************************************
 * search_login:
 *
 *******************************************************************/
static void
search_login(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        char                    *login,
        pin_flist_t             **service_flist,
        poid_t                  **service_poid,
        poid_t                  **account_obj,
        pin_errbuf_t            *ebufp)
{

        // check and clear err buff
        if (PIN_ERR_IS_ERR(ebufp))
            return;
        PIN_ERR_CLEAR_ERR(ebufp);

        // create a search flist
        pin_flist_t *search_flist = PIN_FLIST_CREATE(ebufp);
        pin_flist_t *vp = NULL;

        pin_cookie_t cookie = NULL;
        pin_flist_t * ret_flist = NULL;
        int32 element_id;

        /*
         * Retrieve the database number
         */
        int64 db = PIN_POID_GET_DB(a_pdp);

        /*
         * Create a search object, set flags, template and the arguments
         */
        poid_t * pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
        PIN_FLIST_FLD_PUT(search_flist, PIN_FLD_POID, (void *)pdp, ebufp);

        u_int uint_val = SRCH_DISTINCT;
        PIN_FLIST_FLD_SET(search_flist, PIN_FLD_FLAGS, (void *)&uint_val, ebufp);

        PIN_FLIST_FLD_SET(search_flist, PIN_FLD_TEMPLATE,
                          (void *) "select X from /service where F1 = V1 ", ebufp);

        vp = PIN_FLIST_ELEM_ADD(search_flist, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(vp, PIN_FLD_LOGIN, login, ebufp);

        /*
         * Create PIN_FLD_RESULTS and set a NULL poid in it
         */ 
        vp = PIN_FLIST_ELEM_ADD(search_flist, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(vp, PIN_FLD_POID, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(vp, PIN_FLD_ACCOUNT_OBJ, (void *)NULL, ebufp);
        // note, consider adding a pin_fld_account_obj

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                          "search_login search flist", search_flist);

        /*
         * Call the PCM_OP_SEARCH
         */
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flist, &ret_flist, ebufp);

        /*
         * Use an internal return flist for the PCM call, when we retrieve the fields
         * from PIN_FLD_RESULTS, use TAKE since we are destroying the return flist
         */
        cookie = NULL;
        vp = PIN_FLIST_ELEM_GET_NEXT(ret_flist, PIN_FLD_RESULTS, &element_id, 1, &cookie, ebufp);

        /*
         * In case PCM_OP_SEARCH fails/returns NULL, exit the function
         */
        if (ret_flist == NULL || vp == NULL) {
            PIN_FLIST_DESTROY_EX(&search_flist, NULL);

            if (ret_flist != NULL) {
                PIN_FLIST_DESTROY_EX(&ret_flist, NULL);
            }

            return;
        }

        *service_poid = PIN_FLIST_FLD_TAKE(vp, PIN_FLD_POID, 0, ebufp); 
        *account_obj = PIN_FLIST_FLD_TAKE(vp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp); 

        /*
         * Cleaning
         */
        PIN_FLIST_DESTROY_EX(&search_flist, NULL);
        PIN_FLIST_DESTROY_EX(&ret_flist, NULL);

        if (PIN_ERR_IS_ERR(ebufp)) {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                            "search_login error", ebufp);
        }

        return;
}

static void
fm_nai_act_rate(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        poid_t                  *account_obj,
        char                    *date,
        char                    *descr,
        int32                   *model_code,
        pin_decimal_t           *trans_id,
        char                    *prompt_txt,
        int64                   *tokens_in,
        int64                   *tokens_out,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        /*
         * Some Notes:
         *    use PUT for poids that are from PIN_POID_CREATE
         *    certain functions requires int64, some int32, some needs pointers
         */

        int64 one = 1;
        int64 neg_one = -1;
        int64 zero = 0;

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "ERROR BUFFER NOT EMPTY IN BAL FUNCTION");
                return;
        }
        PIN_ERR_CLEAR_ERR(ebufp);

        /*
         * If convert_to_epoch fails, it will return -1
         */
        int64 timestamp = convert_to_epoch(date);
        if (timestamp == -1) {
            *r_flistpp = PIN_FLIST_CREATE(ebufp);
            poid_t *err_poid = PIN_POID_CREATE(1, "/error", -1, ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, err_poid, ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, (void *) "1", ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, (void *) "Invalid information: date (PIN_FLD_NAME)", ebufp);

            return;
        }

        int64 db = PIN_POID_GET_DB(a_pdp);

        pin_flist_t * cust_flist = PIN_FLIST_CREATE(ebufp);
        poid_t * pdp = NULL;

        // 0 PIN_FLD_POID POID [0] 0.0.0.1 /plan ACCOUNT_NUM 0
        PIN_FLIST_FLD_SET(cust_flist, PIN_FLD_POID, (void *)account_obj, ebufp);

        // 0 PIN_FLD_OBJ_TYPE STR [0] "/usageg3"
        PIN_FLIST_FLD_SET(cust_flist, PIN_FLD_OBJ_TYPE, (void *) "/usageg3", ebufp);

        // 0 PIN_FLD_PROGRAM_NAME STR [0] "load_session.nap"
        PIN_FLIST_FLD_SET(cust_flist, PIN_FLD_PROGRAM_NAME, (void *) "load_session.nap", ebufp);

        // 0 PIN_FLD_FLAGS INT [0] 0
        PIN_FLIST_FLD_SET(cust_flist, PIN_FLD_FLAGS, &zero, ebufp);

        // 0 PIN_FLD_START_T TSTAMP [0] (1756133389)
        PIN_FLIST_FLD_SET(cust_flist, PIN_FLD_START_T, &timestamp, ebufp);
        // 0 PIN_FLD_END_T TSTAMP [0] (1756133389)
        PIN_FLIST_FLD_SET(cust_flist, PIN_FLD_END_T, &timestamp, ebufp);

        // 0 PIN_FLD_DESCR STR [0] "MYDESCR"
        PIN_FLIST_FLD_SET(cust_flist, PIN_FLD_DESCR, (void *)descr, ebufp);

        // 0 PIN_FLD_INHERITED_INFO SUBSTRUCT [0]
        pin_flist_t *inherited_info = PIN_FLIST_SUBSTR_ADD(cust_flist, PIN_FLD_INHERITED_INFO, ebufp);

        // 1     PIN_FLD_INFO_NAI3 SUBSTRUCT [0]
        pin_flist_t *nextaig3 = PIN_FLIST_SUBSTR_ADD(inherited_info, PIN_FLD_INFO_NAI3, ebufp);

        // 2         PIN_FLD_MODELTYPE_NAI3 STR [0] "3.0 or 3.5"
        PIN_FLIST_FLD_SET(nextaig3, PIN_FLD_MODELTYPE_NAI3, (void *)model_code, ebufp);

        // 2         PIN_FLD_TRANSACTION_ID STR [0] "MYTRANSID"
        PIN_FLIST_FLD_SET(nextaig3, PIN_FLD_TRANSACTION_ID, (void *)trans_id, ebufp);

        // 2         PIN_FLD_PROMPT_NAI3 STR [0] "MYPROMPT"
        PIN_FLIST_FLD_SET(nextaig3, PIN_FLD_PROMPT_NAI3, (void *)prompt_txt, ebufp);

        // 2         PIN_FLD_INPUT_TOKENS_NAI3 INT [0] 205000
        PIN_FLIST_FLD_SET(nextaig3, PIN_FLD_INPUT_TOKENS_NAI3, (void *)tokens_in, ebufp);

        // 2         PIN_FLD_OUTPUT_TOKENS_NAI3 INT [0] 200
        PIN_FLIST_FLD_SET(nextaig3, PIN_FLD_OUTPUT_TOKENS_NAI3, (void *)tokens_out, ebufp);

        pin_flist_t * pcm_return_flist = PIN_FLIST_CREATE(ebufp);
        PCM_OP(ctxp, PCM_OP_ACT_LOAD_SESSION, flags, cust_flist, &pcm_return_flist, ebufp); //dont use r_flistpp

        *r_flistpp = PIN_FLIST_CREATE(ebufp);

        /*
         * If theres an error, sends error code 1 with the message
         * else, sends error code 0 and the poid
         */
        if (PIN_ERR_IS_ERR(ebufp)) {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_nai_read error", ebufp);
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_nai_act_rate PCM_OP error", ebufp);

            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, (void *)"1", ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, (void *)"Issue with PCM_OP_ACT_LOAD_SESSION", ebufp);
        } else {
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_ACT_LOAD_SESSION output", pcm_return_flist);

            poid_t *cust_poid = PIN_FLIST_FLD_GET(pcm_return_flist, PIN_FLD_POID, 0, ebufp);
                if (cust_poid != NULL) {
                    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, (void *)cust_poid, ebufp);
            }

            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, (void *) "0", ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, (void *) "Act rated successfully", ebufp);

        }

        PIN_FLIST_DESTROY_EX(&cust_flist, NULL);
        PIN_FLIST_DESTROY_EX(&pcm_return_flist, NULL);
        return;
}
