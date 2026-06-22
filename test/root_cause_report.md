# NextAI G3 — Token Rating Root Cause Report
**Date:** June 21, 2026
**Issue:** NAI-G3-O3-PAYG-Token never produced a rated charge (event_bal_impacts_t)
**Status:** RESOLVED

## Root Cause
BRM on this server runs with `pin_virtual_time` in mode 2 ("running"), with the
virtual clock set ~4 months ahead of real wall-clock time (virtual "now" was
Oct 14, 2026 vs. real date Jun 21, 2026).

All test events were built using real-world June 2026 timestamps for
PIN_FLD_START_T / PIN_FLD_END_T. Although these timestamps were always "in
the past" relative to both real time and virtual time, they were apparently
too far in the past relative to BRM's *virtual* now for the rating engine to
accept them — the event was created successfully every time, but rating
silently produced RATING_STATUS=12 (no balance impact, no RUM match) instead
of RATING_STATUS=0 (success).

## The Fix
Set PIN_FLD_START_T / PIN_FLD_END_T to a timestamp at or near BRM's current
virtual time (queryable via `pin_virtual_time` on the server) rather than
the real wall-clock date.

Check current virtual time:
```bash
pin_virtual_time
```

## Verified Working Test (testnap)
```
r << XXX 1
0 PIN_FLD_POID POID [0] 0.0.0.1 /account 872146 0
0 PIN_FLD_SERVICE_OBJ POID [0] 0.0.0.1 /service/nextaig3 870450 0
0 PIN_FLD_OBJ_TYPE STR [0] "/usageg3"
0 PIN_FLD_PROGRAM_NAME STR [0] "load_session.nap"
0 PIN_FLD_FLAGS INT [0] 0
0 PIN_FLD_START_T TSTAMP [0] (1791952198)
0 PIN_FLD_END_T TSTAMP [0] (1791952198)
0 PIN_FLD_DESCR STR [0] "Virtual-time-matched test"
0 PIN_FLD_INHERITED_INFO SUBSTRUCT [0]
1     PIN_FLD_INFO_NAI3 SUBSTRUCT [0]
2         PIN_FLD_MODELTYPE_NAI3 ENUM [0] 1
2         PIN_FLD_TRANSACTION_ID DECIMAL [0] 55566.00
2         PIN_FLD_PROMPT_NAI3 STR [0] "Virtual time test prompt"
2         10013 INT [0] 1000
2         10014 INT [0] 500
XXX
xop 161 0 1
```

## Result
- Event POID: 364861938562103455
- RATING_STATUS: 0 (success)
- RUM_NAME: Tokens_NAI3
- AMOUNT: $0.10 (1500 tokens / 1000 * $0.05/1000-token rate = correct math)
- TAX_CODE: AIT
- IMPACT_CATEGORY: default
- RESOURCE_ID: 840 (USD)

## Other config fixes applied during investigation (verified harmless/correct,
kept in place):
1. purchased_product_t.usage_start_t corrected from a future date to a past
   date (1767225600 / Jan 1 2026) for all 10 G3 purchased products.
2. rate_bal_impacts_t.impact_category set to 'default' for the Token rate.
3. rate_bal_impacts_t.gl_id set to 104 for the Token rate.

## Dead-end theories ruled out this session (do not re-investigate)
- IFW / wireless.reg pipeline — unrelated, PCM_OP_ACT_LOAD_SESSION rates
  in-line and does not use IFW.
- event_rum_map_t being empty — unused/legacy table on this BRM version,
  empty server-wide for all teams regardless of rating success.
- rate_t.step_resource / type — these encode currency/BEID type, not RUM
  linkage; not the cause.
- EVENT_TYPE on the storable class (USAGE_PREPAID vs NONE) — both values
  exist among working comparator classes; not the cause.
- RUM expression syntax / hidden characters in Tokens_NAI3 — byte-verified
  clean, not the cause.
- G1/G4 custom FM code structure — confirmed functionally equivalent to G3's
  code; not the cause. (Also discovered: G4's currently-loaded module fails
  the exact same way as G3 when tested fresh with an old timestamp, which is
  what proved this was a server-wide timing issue, not a G3-specific bug.)
