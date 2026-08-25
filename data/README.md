# Market Data

This directory contains datasets used for validation.

Datasets are not distributed with the repository.

Expected files:

- AAPL_2013_message_L1.csv
- AAPL_2013_orderbook_L1.csv
- AAPL_2012_message_L5.csv
- AAPL_2012_orderbook_L5.csv

Source:
LOBSTER sample datasets.

## NOTE :
Replay validation was performed against LOBSTER AAPL historical datasets. Synthetic datasets reconstructed exactly. Historical datasets exhibited expected discrepancies due to orders that were already resting in the book before the start of the replay window. Investigation of unmatched deletion and execution events confirmed that many referenced order IDs had no corresponding submission event within the available message stream.