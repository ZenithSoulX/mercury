# Validation Methodology

Mercury was validated using historical NASDAQ LOBSTER data.

Dataset:
- Symbol: AAPL
- Date: 2013
- Depth: L1
- Events: 118,497

Files:
- message.csv
- orderbook.csv

Event Types Observed:
- 1: New Limit Order
- 3: Order Deletion
- 4: Visible Execution
- 5: Hidden Execution

Validation Procedure:
1. Replay every message event.
2. Reconstruct the visible order book.
3. Compare Mercury's best bid/ask against LOBSTER snapshots.
4. Record mismatches.

# First Mismatch 

It is possible that LOBSTER's sample files doesn't necessarily start from a genuinely empty book. 
The trading day begins at market open, and the exchange's book at 9:30AM often already reflects pre-market or opening-auction activity that occurred before the first row of the sample file.

Thus we will be finding the `stabilization point` where our engine's state stabilizes relative to ground truth. Row's before this point reflect the message file's inherent limitation - resting liquidity present at the start of trading day (from pre-market/auction activity) has no corresponding submission event in this data, so any engine that processes documented events cannot reconstruct it.
This is a property of the data window, not a deflect in the matching engine, and is standard when validating against any bounded slice of continous order flow.

## Validation

20+ unit tests covering matching, cancellation, partial fills, reductions, and price-level management.
End-to-end replay validation using a synthetic event stream exercising submissions, executions deletions, and partial cancellations.
Evaluated against multiple LOBSTER datasets (Level 1 and Level 5). Observed reconstruction divergence from row 0 due to missing initial book state and references to pre-existing liquidity.