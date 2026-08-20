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