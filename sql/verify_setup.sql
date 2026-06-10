-- ============================================================
-- NextAI BRM Group 03 - Verification Queries
-- Run against: Oracle DB, service orclpdb, user pin/pin
-- ============================================================


-- ------------------------------------------------------------
-- 1. Verify product catalog (plans, deals, products)
-- ------------------------------------------------------------
SELECT 'PLAN'    AS object_type, POID_ID0, NAME FROM PIN.PLAN_T    WHERE NAME LIKE '%NAI%'
UNION ALL
SELECT 'DEAL'    AS object_type, POID_ID0, NAME FROM PIN.DEAL_T    WHERE NAME LIKE '%NAI%'
UNION ALL
SELECT 'PRODUCT' AS object_type, POID_ID0, NAME FROM PIN.PRODUCT_T WHERE NAME LIKE '%NAI%'
ORDER BY object_type, NAME;


-- ------------------------------------------------------------
-- 2. Verify all 3 customer accounts with billing details
-- ------------------------------------------------------------
SELECT
    a.POID_ID0          AS account_poid,
    a.ACCOUNT_NO,
    a.STATUS,
    b.POID_ID0          AS billinfo_poid,
    b.PAY_TYPE,
    bg.POID_ID0         AS bal_grp_poid
FROM PIN.ACCOUNT_T a
JOIN PIN.BILLINFO_T      b  ON b.ACCOUNT_OBJ_ID0  = a.POID_ID0
JOIN PIN.BALANCE_GROUP_T bg ON bg.ACCOUNT_OBJ_ID0 = a.POID_ID0
WHERE a.POID_ID0 IN (357003, 367997, 371916);


-- ------------------------------------------------------------
-- 3. Verify /service/nextai instances per account
-- ------------------------------------------------------------
SELECT
    s.ACCOUNT_OBJ_ID0   AS account_poid,
    s.POID_ID0          AS service_poid,
    s.POID_TYPE         AS service_type,
    s.LOGIN
FROM PIN.SERVICE_T s
WHERE s.POID_TYPE = '/service/nextai'
ORDER BY s.ACCOUNT_OBJ_ID0;


-- ------------------------------------------------------------
-- 4. Verify purchased products per account
-- ------------------------------------------------------------
SELECT
    pp.ACCOUNT_OBJ_ID0  AS account_poid,
    pp.POID_ID0         AS purchased_product_poid,
    pp.NAME             AS product_name,
    pp.STATUS
FROM PIN.PURCHASED_PRODUCT_T pp
WHERE pp.ACCOUNT_OBJ_ID0 IN (357003, 367997, 371916)
ORDER BY pp.ACCOUNT_OBJ_ID0, pp.NAME;


-- ------------------------------------------------------------
-- 5. Full summary: account + service + product in one view
-- ------------------------------------------------------------
SELECT
    a.ACCOUNT_NO,
    s.LOGIN,
    s.POID_TYPE         AS service_type,
    pp.NAME             AS purchased_product
FROM PIN.ACCOUNT_T a
JOIN PIN.SERVICE_T           s  ON s.ACCOUNT_OBJ_ID0  = a.POID_ID0
JOIN PIN.PURCHASED_PRODUCT_T pp ON pp.ACCOUNT_OBJ_ID0 = a.POID_ID0
WHERE s.POID_TYPE = '/service/nextai'
  AND pp.NAME LIKE '%NAI%'
ORDER BY a.ACCOUNT_NO;
