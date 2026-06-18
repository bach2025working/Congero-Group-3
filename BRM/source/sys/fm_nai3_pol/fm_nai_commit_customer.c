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
 * Contains the NAI_OP_COMMIT_CUSTOMER operation.
 *******************************************************************/

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#define FILE_LOGNAME "fm_nai_commit_customer.c(1)"

#include "ops/nai_cust_ops.h"
#include "cm_fm.h"
#include "pin_errs.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_nai_commit_customer(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void search_plan(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        char                    *code,
        pin_flist_t             **plan_flist,
        poid_t                  **plan_poid,
        pin_errbuf_t            *ebufp);

static void search_deal(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        char                    *code,
        pin_flist_t             **deal_flist,
        poid_t                  **deal_poid,
        pin_errbuf_t            *ebufp);

static void
fm_nai_commit_customer(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        char                    *passwd_clear,
        char                    *login,
        char                    *code,
        char                    *last_name,
        char                    *first_name,
        char                    *email_addr,
        char                    *country,
        char                    *zip,
        char                    *state,
        char                    *city,
        char                    *address,
        char                    *name,
        char                    *delivery_descr,
        poid_t                  *plan_poid,
        poid_t                  *deal_poid,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/


/*******************************************************************
 * Main routine for the NAI_OP_COMMIT_CUSTOMER operation.
 *******************************************************************/
void
op_nai_commit_customer(
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
        if (opcode != NAI_OP_COMMIT_CUSTOMER) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_nai_commit_customer opcode error", ebufp);

                return;
        }

        /***********************************************************
         * Debut what we got.
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_nai_commit_customer input flist", i_flistp);

        /***********************************************************
         * Main rountine for this opcode
         ***********************************************************/

        char err_msg[256] = "Missing fields: ";
        int offset = strlen(err_msg);
        int missing_fields = 0;

        /*
         *  Params:
         *    field: name of the field to be retrieved
         *    var: variable to hold the return value (NULL if missing)
         *    label: label of the field to add to error message
         *  This macro takes the above params and check their existence, if not, it will set missing_fields to 1
         *  and append the label to the error message
         */
        #define CHECK_AND_GET(field, var, label) \
            var = PIN_FLIST_FLD_GET(i_flistp, field, 1, ebufp); \
            if (var == NULL) { \
                if (offset < sizeof(err_msg) - 1) { \
                    if (missing_fields) offset += snprintf(err_msg + offset, sizeof(err_msg) - offset, ", "); \
                    offset += snprintf(err_msg + offset, sizeof(err_msg) - offset, "%s", label); \
                } \
                missing_fields = 1; \
            }

        /*
         * Checking the existence of all required fields
         */
        char *passwd_clear = NULL;
        char *login         = NULL;
        char *code          = NULL;
        char *last_name     = NULL;
        char *first_name    = NULL;
        char *email_addr    = NULL;
        char *country       = NULL;
        char *zip           = NULL;
        char *state         = NULL;
        char *city          = NULL;
        char *address       = NULL;
        char *name          = NULL;
        char *delivery_descr= NULL;

        CHECK_AND_GET(PIN_FLD_PASSWD_CLEAR,  passwd_clear,  "PASSWD_CLEAR");
        CHECK_AND_GET(PIN_FLD_LOGIN,         login,         "LOGIN");
        CHECK_AND_GET(PIN_FLD_CODE,          code,          "CODE");
        CHECK_AND_GET(PIN_FLD_LAST_NAME,     last_name,     "LAST_NAME");
        CHECK_AND_GET(PIN_FLD_FIRST_NAME,    first_name,    "FIRST_NAME");
        CHECK_AND_GET(PIN_FLD_EMAIL_ADDR,    email_addr,    "EMAIL_ADDR");
        CHECK_AND_GET(PIN_FLD_COUNTRY,       country,       "COUNTRY");
        CHECK_AND_GET(PIN_FLD_ZIP,           zip,           "ZIP");
        CHECK_AND_GET(PIN_FLD_STATE,         state,         "STATE");
        CHECK_AND_GET(PIN_FLD_CITY,          city,          "CITY");
        CHECK_AND_GET(PIN_FLD_ADDRESS,       address,       "ADDRESS");
        CHECK_AND_GET(PIN_FLD_NAME,          name,          "NAME");
        CHECK_AND_GET(PIN_FLD_DELIVERY_DESCR,delivery_descr,"DELIVERY_DESCR");

        /*
         * If there is a missing field, we return error code 1 with a list of missing fields
         * Note: destroy the input flist since we're returning
         */ 
        if (missing_fields) {
            pin_flist_t *r_flistp = PIN_FLIST_CREATE(ebufp);
            poid_t *err_poid = PIN_POID_CREATE(1, "/error", -1, ebufp);
            PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, err_poid, ebufp);
            PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE,
                              (void *)"2: missing information for customer", ebufp);
            PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
                              (void *)err_msg, ebufp);
            *r_flistpp = r_flistp;

            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, err_msg, ebufp);
            PIN_FLIST_DESTROY_EX(&i_flistp, ebufp);
            return;
        }

        PIN_ERR_CLEAR_ERR(ebufp); // clear error buffer before proceeding

        /*
         * PIN_FLD_ACCOUNT_OBJ is not a required fields, but we use it to retrive the database number for the poids
         */ 
        poid_t * a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

        /*
         * plan_flist will hold the PIN_FLD_RESULTS 
         * and plan_poid will hold the poid of the plan (via TAKE)
         */ 
        void * plan_poid = NULL;
        pin_flist_t * plan_flist = NULL;
        search_plan(ctxp, flags, a_pdp, code, &plan_flist, &plan_poid, ebufp);

        void * deal_poid = NULL;
        pin_flist_t * deal_flist = NULL;
        search_deal(ctxp, flags, a_pdp, code, &deal_flist, &deal_poid, ebufp);

        /*
         * Calling our commit_cusomter
         */ 
        fm_nai_commit_customer(ctxp, flags, a_pdp,
                               passwd_clear, login, code, last_name, first_name,
                               email_addr, country, zip, state, city,
                               address, name, delivery_descr,
                               plan_poid, deal_poid,
                               r_flistpp, ebufp);

        /***********************************************************
         * Error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_FLIST_DESTROY_EX(r_flistpp, ebufp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_nai_commit_customer error", ebufp);
        } else {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_nai_commit_customer output flist", *r_flistpp);
        }

        PIN_FLIST_DESTROY_EX(&i_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&plan_flist, ebufp);
        PIN_FLIST_DESTROY_EX(&deal_flist, ebufp);

        return;
}


/*******************************************************************
 * search_plan:
 *
 *******************************************************************/
static void
search_plan(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        char                    *code,
        pin_flist_t             **plan_flist,
        poid_t                  **plan_poid,
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
                          (void *) "select X from /plan where F1 = V1 ", ebufp);

        vp = PIN_FLIST_ELEM_ADD(search_flist, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(vp, PIN_FLD_CODE, (void *)code, ebufp);

        /*
         * Create PIN_FLD_RESULTS and set a NULL poid in it
         */ 
        vp = PIN_FLIST_ELEM_ADD(search_flist, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(vp, PIN_FLD_POID, (void *)NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                          "search_plan search flist", search_flist);

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
        *plan_poid = PIN_FLIST_FLD_TAKE(vp, PIN_FLD_POID, 0, ebufp); 

        /*
         * Cleaning
         */
        PIN_FLIST_DESTROY_EX(&search_flist, NULL);
        PIN_FLIST_DESTROY_EX(&ret_flist, NULL);

        if (PIN_ERR_IS_ERR(ebufp)) {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                            "search_plan error", ebufp);
        }

        return;
}


/*******************************************************************
 * search_deal:
 *
 *******************************************************************/
static void
search_deal(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        char                    *code,
        pin_flist_t             **deal_flist,
        poid_t                  **deal_poid,
        pin_errbuf_t            *ebufp)
{

        if (PIN_ERR_IS_ERR(ebufp))
            return;
        PIN_ERR_CLEAR_ERR(ebufp);

        pin_flist_t *search_flist = PIN_FLIST_CREATE(ebufp);
        pin_flist_t *vp = NULL;

        pin_cookie_t cookie = NULL;
        pin_flist_t * ret_flist = NULL;
        int32 element_id;

        int64 db = PIN_POID_GET_DB(a_pdp);

        poid_t * pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
        PIN_FLIST_FLD_PUT(search_flist, PIN_FLD_POID, (void *)pdp, ebufp);

        u_int uint_val = SRCH_DISTINCT;
        PIN_FLIST_FLD_SET(search_flist, PIN_FLD_FLAGS, (void *)&uint_val, ebufp);

        PIN_FLIST_FLD_SET(search_flist, PIN_FLD_TEMPLATE,
                          (void *) "select X from /deal where F1 = V1 ", ebufp);

        vp = PIN_FLIST_ELEM_ADD(search_flist, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(vp, PIN_FLD_CODE, (void *)code, ebufp);

        vp = PIN_FLIST_ELEM_ADD(search_flist, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(vp, PIN_FLD_POID, (void *)NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                          "search_deal search flist", search_flist);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flist, &ret_flist, ebufp);

        cookie = NULL;
        vp = PIN_FLIST_ELEM_GET_NEXT(ret_flist, PIN_FLD_RESULTS, &element_id, 1, &cookie, ebufp);
        *deal_poid = PIN_FLIST_FLD_TAKE(vp, PIN_FLD_POID, 0, ebufp); 

        PIN_FLIST_DESTROY_EX(&search_flist, NULL);
        PIN_FLIST_DESTROY_EX(&ret_flist, NULL);

        if (PIN_ERR_IS_ERR(ebufp)) {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                            "search_deal error", ebufp);
        }
        else {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "FINALLY ESCAPE SEARCH DEAL");
        }

        return;
}

static void
fm_nai_commit_customer(
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *a_pdp,
        char                    *passwd_clear,
        char                    *login,
        char                    *code,
        char                    *last_name,
        char                    *first_name,
        char                    *email_addr,
        char                    *country,
        char                    *zip,
        char                    *state,
        char                    *city,
        char                    *address,
        char                    *name,
        char                    *delivery_descr,
        poid_t                  *plan_poid,
        poid_t                  *deal_poid,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        /*
         * Some Notes:
         *    use PUT for poids that are from PIN_POID_CREATE
         *    certain functions requires int64, some int32, some needs pointers
         */

        poid_t          *order_poidp = NULL;
        char            *poid_type = NULL;

        int64 one = 1;
        int64 neg_one = -1;
        int64 zero = 0;

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "ERROR BUFFER NOT EMPTY IN COMMIT FUNCTION");
                return;
        }
        PIN_ERR_CLEAR_ERR(ebufp);

        int64 db = PIN_POID_GET_DB(a_pdp);

        pin_flist_t * cust_flist = PIN_FLIST_CREATE(ebufp);
        poid_t * pdp = NULL;

        // 0 PIN_FLD_POID POID [0] 0.0.0.1 /plan PLAN_NUM 0
        PIN_FLIST_FLD_SET(cust_flist, PIN_FLD_POID, (void *)plan_poid, ebufp);

        // SERVICES
        // 0 PIN_FLD_services ARRAY [0]
        pin_flist_t * services = PIN_FLIST_ELEM_ADD(cust_flist, PIN_FLD_SERVICES, 0, ebufp);

        // 1 PIN_FLD_services_OBJ POID [0] 0.0.0.1 /service/nextai -1 0
        pdp = PIN_POID_CREATE(db, "/service/nextai", neg_one, ebufp);
        PIN_FLIST_FLD_PUT(services, PIN_FLD_SERVICE_OBJ, (void *)pdp, ebufp);

        // 1 PIN_FLD_PASSWD_CLEAR STR [0] "MY_PW"
        PIN_FLIST_FLD_SET(services, PIN_FLD_PASSWD_CLEAR, (void *)passwd_clear, ebufp);

        // 1 PIN_FLD_LOGIN STR [0] "MY_LOGIN"
        PIN_FLIST_FLD_SET(services, PIN_FLD_LOGIN, (void *)login, ebufp);

        // DEALS
        // 1 PIN_FLD_DEALS ARRAY [0]
        pin_flist_t * deals = PIN_FLIST_ELEM_ADD(services, PIN_FLD_DEALS, 0, ebufp);

        // 2 PIN_FLD_CODE STR [0] "DEAL_CODE"
        PIN_FLIST_FLD_SET(deals, PIN_FLD_CODE, (void *)code, ebufp);

        // 2 PIN_FLD_ACCOUNT_OBJ POID [0] 0.0.0.1 /account 1 0
        pdp = PIN_POID_CREATE(db, "/account", (int64)1, ebufp);
        PIN_FLIST_FLD_PUT(deals, PIN_FLD_ACCOUNT_OBJ, (void *)pdp, ebufp);

        // 2 PIN_FLD_DEAL_OBJ POID [0] 0.0.0.1 /deal DEAL_NUM 0
        PIN_FLIST_FLD_SET(deals, PIN_FLD_DEAL_OBJ, (void *)deal_poid, ebufp);

        // NAMEINFO
        // 0 PIN_FLD_NAMEINFO ARRAY [1]
        pin_flist_t * nameinfo = PIN_FLIST_ELEM_ADD(cust_flist, PIN_FLD_NAMEINFO, 1, ebufp);

        // 1 PIN_FLD_LAST_NAME STR [0] "MY_LAST"
        PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_LAST_NAME, (void *)last_name, ebufp);
        // 1 PIN_FLD_FIRST_NAME STR [0] "MY_FIRST"
        PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_FIRST_NAME, (void *)first_name, ebufp);
        // 1 PIN_FLD_CONTACT_TYPE STR [0] "Account holder"
        PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_CONTACT_TYPE, (void *) "Account holder", ebufp);
        // 1 PIN_FLD_EMAIL_ADDR STR [0] "MY_EMAIL@gmail.com"
        PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_EMAIL_ADDR, (void *)email_addr, ebufp);
        // 1 PIN_FLD_COUNTRY STR [0] "USA"
        PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_COUNTRY, (void *)country, ebufp);
        // 1 PIN_FLD_ZIP STR [0] "MY_ZIP"
        PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_ZIP, (void *)zip, ebufp);
        // 1 PIN_FLD_STATE STR [0] "MY_STATE"
        PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_STATE, (void *)state, ebufp);
        // 1 PIN_FLD_CITY STR [0] "MY_CITY"
        PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_CITY, (void *)city, ebufp);
        // 1 PIN_FLD_ADDR STR [0] "MY_ADDR"
        PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_ADDRESS, (void *)address, ebufp);

        // PAYINFO
        // 0 PIN_FLD_PAYINFO ARRAY [0] allocated 20, used 6
        pin_flist_t * payinfo = PIN_FLIST_ELEM_ADD(cust_flist, PIN_FLD_PAYINFO, 2, ebufp);

        // 1 PIN_FLD_NAME STR [0] "Invoice1"
        PIN_FLIST_FLD_SET(payinfo, PIN_FLD_NAME, (void *) "Invoice1", ebufp);
        // 1 PIN_FLD POID POID [0] 0.0.0.1 /payinfo/invoice -1 0
        pdp = PIN_POID_CREATE(db, "/payinfo/invoice", neg_one, ebufp);
        PIN_FLIST_FLD_PUT(payinfo, PIN_FLD_POID, (void *)pdp, ebufp);

        // INHERITED INFO
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "STARTING INHERIT INFO");
        // PIN_FLD_INHERITED_INFO SUBSTRUCT [0]
        pin_flist_t * in_info = PIN_FLIST_ELEM_ADD(payinfo, PIN_FLD_INHERITED_INFO, 0, ebufp);

        // INV_INFO
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "STARTING INV INFO");
        // 2 PIN_FLD_INV_INFO ARRAY [0]
        pin_flist_t * inv_info = PIN_FLIST_ELEM_ADD(in_info, PIN_FLD_INV_INFO, 0, ebufp);

        // 3 PIN_FLD_DELIVERY_PREFER ENUM [0] 0
        int64 inv_zero = 0;
        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_PREFER, &inv_zero, ebufp);
        // 3 PIN_FLD_NAME STR [0] "MY_FIRST MY_LAST"
        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_NAME, (void *)last_name, ebufp);
        // 3 PIN_FLD_INV_TERMS ENUM [0] 0
        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_INV_TERMS, &zero, ebufp);
        // 3 PIN_FLD_DEKUVERT_DESCR STR [0] "MY_EMAIL@gmail.com"
        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_DESCR, (void *)email_addr, ebufp);
        // 3 PIN_FLD_EMAIL_ADDR STR [0] ""
        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_EMAIL_ADDR, NULL, ebufp);
        // 3 PIN_FLD_COUNTRY STR [0] "USA"
        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_COUNTRY, (void *)country, ebufp);
        // 3 PIN_FLD_ZIP STR [0] "MY_ZIP"
        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_ZIP, (void *)zip, ebufp);
        // 3 PIN_FLD_STATE STR [0] "MY_STATE"
        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_STATE, (void *)state, ebufp);
        // 3 PIN_FLD_CITY STR [0] "MY_CITY"
        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_CITY, (void *)city, ebufp);
        // 3 PIN_FLD_ADDRESS STR [0] "MY_ADDRESS"
        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_ADDRESS, (void *)address, ebufp);

        // 1 PIN_FLD_PAY_TYPE ENUM [0] 10001
        int64 paytype = 10001;
        PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAY_TYPE, &paytype, ebufp);

        // ACCTINFO
        // 0 PIN_FLD_ACCTINFO ARRAY [0] allocated 20, used 4
        pin_flist_t * acct_info = PIN_FLIST_ELEM_ADD(cust_flist, PIN_FLD_ACCTINFO, 0, ebufp);

        // 1 PIN_FLD_POID POID [0] 0.0.0.1 /account -1 0
        pdp = PIN_POID_CREATE(db, "/account", neg_one, ebufp);
        PIN_FLIST_FLD_PUT(acct_info, PIN_FLD_POID, (void *)pdp, ebufp);
        // 1 PIN_FLD_BAL_INFO ARRAY [0]     NULL array ptr
        PIN_FLIST_FLD_SET(acct_info, PIN_FLD_BAL_INFO, NULL, ebufp);
        // 1 PIN_FLD_CURRENCY INT [0] 840
        int64 currency = 840;
        PIN_FLIST_FLD_SET(acct_info, PIN_FLD_CURRENCY, &currency, ebufp);
        // 1 PIN_FLD_BUSINESS_TYPE ENUM [0] 1

        int32 businesstype = 1;
        PIN_FLIST_FLD_SET(acct_info, PIN_FLD_BUSINESS_TYPE, &businesstype, ebufp);

        pin_flist_t * pcm_return_flist = PIN_FLIST_CREATE(ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "CALLING PCM_OP_CUST_COMMIT_CUSTOMER");
        PCM_OP(ctxp, PCM_OP_CUST_COMMIT_CUSTOMER, flags, cust_flist, &pcm_return_flist, ebufp); //dont use r_flistpp

        *r_flistpp = PIN_FLIST_CREATE(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_nai_commit_customer output flist", *r_flistpp);

        /*
         * If theres an error, sends error code 1 with the message
         * else, sends error code 0 and the poid
         */
        if (PIN_ERR_IS_ERR(ebufp)) {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "ERROR AFTER CALLING PCM COMMIT CUS");
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_nai_read error", ebufp);
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_nai_commit_customer PCM_OP error", ebufp);

            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, (void *)"1", ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, (void *)"Issue with PCM_OP_CUST_COMMIT_CUSTOMER", ebufp);
        } else {
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CUST_COMMIT_CUSTOMER output", pcm_return_flist);

            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, (void *)"0", ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, (void *)"Customer committed successfully", ebufp);

            poid_t *cust_poid = PIN_FLIST_FLD_GET(pcm_return_flist, PIN_FLD_POID, 0, ebufp);
                if (cust_poid != NULL) {
                    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, (void *)cust_poid, ebufp);
            } else {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "No POID returned from PCM_OP_CUST_COMMIT_CUSTOMER");
            }
        }

        PIN_FLIST_DESTROY_EX(&cust_flist, NULL);
        PIN_FLIST_DESTROY_EX(&pcm_return_flist, NULL);
        return;
}
