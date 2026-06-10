**Create base account in Billing Care**

Go to `localhost:7001/bc` → New Account. Fill in contact info, pick any plan that shows in the list, submit. Copy the account POID.

**Get account info from SQL**
```sql
SELECT BAL_GRP_OBJ_ID0 FROM PIN.ACCOUNT_T WHERE POID_ID0 = <account_poid>;
SELECT POID_ID0 FROM PIN.BILLINFO_T WHERE ACCOUNT_OBJ_ID0 = <account_poid>;
```

**Create /service/nextai via opcode 52**
```bash
cat > /tmp/create_svc.flist << 'EOF'
0 PIN_FLD_POID           POID [0] 0.0.0.1 /service/nextai -1 0
0 PIN_FLD_PROGRAM_NAME    STR [0] "testnap"
0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account <account_poid> 0
0 PIN_FLD_SERVICE_ID      STR [0] ""
0 PIN_FLD_LOGIN           STR [0] "<login>@nextai.g03"
0 PIN_FLD_PASSWD_CLEAR    STR [0] "Pin1234!"
0 PIN_FLD_PASSWD_STATUS   ENUM [0] 0
0 PIN_FLD_START_T      TSTAMP [0] (0) <null>
0 PIN_FLD_END_T        TSTAMP [0] (0) <null>
0 PIN_FLD_BAL_GRP_OBJ    POID [0] 0.0.0.1 /balance_group <bal_grp_poid> 0
EOF
```
In testnap:
```
r /tmp/create_svc.flist 1
xop 52 0 1
d
```
Copy the service POID returned.

**Purchase NextAI deal via opcode 108**
```bash
cat > /tmp/purchase_deal.flist << 'EOF'
0 PIN_FLD_POID           POID [0] 0.0.0.1 /account <account_poid> 0
0 PIN_FLD_PROGRAM_NAME    STR [0] "testnap"
0 PIN_FLD_START_T      TSTAMP [0] (0) <null>
0 PIN_FLD_END_T        TSTAMP [0] (0) <null>
0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/nextai <service_poid> 0
0 PIN_FLD_BILLINFO_OBJ   POID [0] 0.0.0.1 /billinfo <billinfo_poid> 0
0 PIN_FLD_DEAL_INFO    SUBSTRUCT [0]
1     PIN_FLD_POID           POID [0] 0.0.0.1 /deal <deal_poid> 0
1     PIN_FLD_NAME            STR [0] "<deal_name>"
1     PIN_FLD_START_T      TSTAMP [0] (0) <null>
1     PIN_FLD_END_T        TSTAMP [0] (0) <null>
1     PIN_FLD_FLAGS           INT [0] 0
1     PIN_FLD_CREDIT_LIMIT_FLAGS    INT [0] 64
1     PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan <plan_poid> 0
1     PIN_FLD_PRODUCTS      ARRAY [0]
2         PIN_FLD_PRODUCT_OBJ    POID [0] 0.0.0.1 /product <product_poid> 0
2         PIN_FLD_NAME            STR [0] "<product_name>"
2         PIN_FLD_DESCR           STR [0] "<description>"
2         PIN_FLD_STATUS         ENUM [0] 1
2         PIN_FLD_STATUS_FLAGS    INT [0] 0
2         PIN_FLD_FLAGS           INT [0] 0
2         PIN_FLD_QUANTITY     DECIMAL [0] 1
2         PIN_FLD_MODE           ENUM [0] 0
2         PIN_FLD_RENEWAL_MODE    INT [0] 0
2         PIN_FLD_PURCHASE_DISCOUNT DECIMAL [0] 0
2         PIN_FLD_CYCLE_DISCOUNT DECIMAL [0] 0
2         PIN_FLD_USAGE_DISCOUNT DECIMAL [0] 0
2         PIN_FLD_BASE_PRODUCT_OBJ   POID [0] 0.0.0.0  0 0
2         PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan <plan_poid> 0
2         PIN_FLD_OWN_MIN      DECIMAL [0] NULL
2         PIN_FLD_OWN_MAX      DECIMAL [0] NULL
2         PIN_FLD_GRACE_PERIOD_OFFSET    INT [0] 0
2         PIN_FLD_GRACE_PERIOD_UNIT    INT [0] 0
2         PIN_FLD_PURCHASE_START_DETAILS    INT [0] 1
2         PIN_FLD_PURCHASE_END_DETAILS    INT [0] 2
2         PIN_FLD_CYCLE_START_DETAILS    INT [0] 1
2         PIN_FLD_CYCLE_END_DETAILS    INT [0] 2
2         PIN_FLD_USAGE_START_DETAILS    INT [0] 1
2         PIN_FLD_USAGE_END_DETAILS    INT [0] 2
2         PIN_FLD_PURCHASE_START_T TSTAMP [0] (0) <null>
2         PIN_FLD_PURCHASE_END_T TSTAMP [0] (0) <null>
2         PIN_FLD_CYCLE_START_T TSTAMP [0] (0) <null>
2         PIN_FLD_CYCLE_END_T  TSTAMP [0] (0) <null>
2         PIN_FLD_USAGE_START_T TSTAMP [0] (0) <null>
2         PIN_FLD_USAGE_END_T  TSTAMP [0] (0) <null>
EOF
```
In testnap:
```
r /tmp/purchase_deal.flist 1
xop 108 0 1
d
```

**3 accounts and POIDs for reference**

| Plan | Account | Bal Group | Billinfo | Service | Deal | Plan POID | Product |
|------|---------|-----------|----------|---------|------|-----------|---------|
| FlatRate | 357003 | 359819 | 360075 | 360834 | 367293 | 367854 | 364910 |
| PAYG | 367997 | 365821 | 366461 | 363197 | 371444 | 372468 | 372596 |
| Monthly | 371916 | 370124 | 370380 | 364223 | 353384 | 370164 | 366318 |

**Verify an account is fully set up:**
```sql
SELECT POID_ID0, NAME FROM PIN.PURCHASED_PRODUCT_T WHERE ACCOUNT_OBJ_ID0 = <account_poid>;
SELECT POID_ID0, POID_TYPE, LOGIN FROM PIN.SERVICE_T WHERE ACCOUNT_OBJ_ID0 = <account_poid>;
```
