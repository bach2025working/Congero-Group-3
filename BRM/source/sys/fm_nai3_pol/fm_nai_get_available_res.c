
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
 * Contains the NAI3_OP_GET_AVAILABLE_RES operation.
 *******************************************************************/

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include "ops/bal.h"
#include <pinlog.h>

#define FILE_LOGNAME "fm_nai_get_available_res.c(1)"

#include "ops/naig3_custom_ops.h"
#include "cm_fm.h"
#include "pin_errs.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_nai_get_available_res(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

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
fm_nai_get_available_res(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        poid_t                  *account_obj,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/


/*******************************************************************
 * Main routine for the NAI3_OP_GET_AVAILABLE_RES operation.
 *******************************************************************/
void
op_nai_get_available_res(
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
        if (opcode != NAI3_OP_GET_AVAILABLE_RES) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_nai_get_available_res opcode error", ebufp);

                return;
        }

        /***********************************************************
         * Debut what we got.
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_nai_get_available_res input flist", i_flistp);

        /***********************************************************
         * Main rountine for this opcode
         ***********************************************************/

        char err_msg[256] = "Login not found";

        /*
         * PIN_FLD_ACCOUNT_OBJ is not a required fields, but we use it to retrive the database number for the poids
         */ 
        poid_t * a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        char * login = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 0, ebufp);

        /*
         * service_flist will hold the PIN_FLD_RESULTS 
         * and service_poid will hold the poid of the service (via TAKE)
         */ 
        poid_t *service_poid = NULL;
        poid_t *account_obj = NULL;
        pin_flist_t * service_flist = NULL;
        search_login(ctxp, flags, a_pdp, login, &service_flist, &service_poid, &account_obj, ebufp);
        // note, may need to return a poid for account_obj as well

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
            PIN_FLIST_DESTROY_EX(&i_flistp, ebufp);
            return;
        }

        /*
         * Calling our get_available_res
         */ 
        fm_nai_get_available_res(ctxp, flags, a_pdp, account_obj, r_flistpp, ebufp);

        /***********************************************************
         * Error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_FLIST_DESTROY_EX(r_flistpp, ebufp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_nai_get_available_res error", ebufp);
        } else {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_nai_get_available_res output flist", *r_flistpp);
        }

        PIN_FLIST_DESTROY_EX(&i_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&service_flist, ebufp);

        return;
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
fm_nai_get_available_res(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        poid_t                  *account_obj,
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

        int64 db = PIN_POID_GET_DB(a_pdp);

        pin_flist_t * cust_flist = PIN_FLIST_CREATE(ebufp);
        poid_t * pdp = NULL;

        // 0 PIN_FLD_POID POID [0] 0.0.0.1 /plan ACCOUNT_NUM 0
        PIN_FLIST_FLD_SET(cust_flist, PIN_FLD_POID, (void *)account_obj, ebufp);

        // BALANCES
        // 0 PIN_FLD_BALANCES ARRAY [0]
        pin_flist_t * balances = PIN_FLIST_ELEM_ADD(cust_flist, PIN_FLD_BALANCES, 0, ebufp);

        // 1 PIN_FLD_THRESHOLDS ARRAY [0]
        pin_flist_t * thresholds = PIN_FLIST_ELEM_ADD(balances, PIN_FLD_THRESHOLDS, 0, ebufp);

        // 2 PIN_FLD_THRESHOLD INT [0] 0
        PIN_FLIST_FLD_SET(thresholds, PIN_FLD_THRESHOLD, &zero, ebufp);
        
        // 2 PIN_FLD_AMOUNT DECIMAL [0] NULL
        PIN_FLIST_FLD_SET(thresholds, PIN_FLD_AMOUNT, NULL, ebufp);
        
        // 1 PIN_FLD_CURRENT_BAL DECIMAL [0] NULL
        current_bal = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(balance_flist, PIN_FLD_CURRENT_BAL, 1, ebufp);
        
        // 1 PIN_FLD_GRANTED_BAL DECIMAL [0] NULL
        granted_bal = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(balance_flist, PIN_FLD_GRANTED_BAL, 1, ebufp);
        
        // 1 PIN_FLD_RESOURCE_ID INT [0] NULL
        PIN_FLIST_FLD_SET(balances, PIN_FLD_RESOURCE_ID, NULL, ebufp);

        pin_flist_t * pcm_return_flist = PIN_FLIST_CREATE(ebufp);
        PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES, flags, cust_flist, &pcm_return_flist, ebufp); //dont use r_flistpp

        *r_flistpp = PIN_FLIST_CREATE(ebufp);

        if (current_bal != NULL) {
                PIN_FLIST_FLD_PUT(usd, PIN_FLD_CURRENT_BAL, current_bal, ebufp);
        }
        
        if (granted_bal != NULL) {
                PIN_FLIST_FLD_PUT(usd, PIN_FLD_GRANTED_BAL, granted_bal, ebufp);
        }

        /*
         * If theres an error, sends error code 1 with the message
         * else, sends error code 0 and the poid
         */
        if (PIN_ERR_IS_ERR(ebufp)) {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_nai_read error", ebufp);
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_nai_get_available_res PCM_OP error", ebufp);

            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, (void *)"1", ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, (void *)"Issue with PCM_OP_BAL_GET_BALANCES", ebufp);
        } else {
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CUST_get_available_res output", pcm_return_flist);

            poid_t *cust_poid = PIN_FLIST_FLD_GET(pcm_return_flist, PIN_FLD_POID, 0, ebufp);
                if (cust_poid != NULL) {
                    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, (void *)cust_poid, ebufp);
            }

            pin_decimal_t * current_bal = NULL;
            pin_decimal_t * granted_bal = NULL;
            pin_flist_t * balance_flist = NULL;

            pin_cookie_t cookie = NULL;

            int32 element_id = 0;
            int64 curr = 0;
            pin_flist_t * usd = NULL;

            /*
             * Walk through the return FLIST and read the balances, use variable curr to keep track of the number of sub-balances
             */
            balance_flist = PIN_FLIST_ELEM_GET_NEXT(pcm_return_flist, PIN_FLD_BALANCES, &element_id, 1, &cookie, ebufp);
            while (balance_flist != NULL) {

                current_bal = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(balance_flist, PIN_FLD_CURRENT_BAL, 0, ebufp);
                granted_bal = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(balance_flist, PIN_FLD_GRANTED_BAL, 0, ebufp);

                usd = PIN_FLIST_ELEM_ADD(*r_flistpp, PIN_FLD_BALANCES, curr, ebufp);
                PIN_FLIST_FLD_SET(usd, PIN_FLD_CURRENT_BAL, (void *)current_bal, ebufp);
                PIN_FLIST_FLD_SET(usd, PIN_FLD_GRANTED_BAL, (void *)granted_bal, ebufp);

                balance_flist = PIN_FLIST_ELEM_GET_NEXT(pcm_return_flist, PIN_FLD_BALANCES, &element_id, 1, &cookie, ebufp);
                curr = curr + 1;
            }
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, (void *) "0", ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, (void *) "Resources obtained successfully", ebufp);

        }

        PIN_FLIST_DESTROY_EX(&cust_flist, NULL);
        PIN_FLIST_DESTROY_EX(&pcm_return_flist, NULL);
        return;
}
