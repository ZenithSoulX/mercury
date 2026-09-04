# Market Data

This directory contains datasets used for validation.

Datasets are not distributed with the repository.

Expected files:

- AAPL_2012-06-21_34200000_57600000_message_1.csv
- AAPL_2012-06-21_34200000_57600000_orderbook_1.csv
- AAPL_2012-06-21_34200000_57600000_message_5.csv
- AAPL_2012-06-21_34200000_57600000_orderbook_5.csv

Source:
LOBSTER sample datasets.

## NOTE :
Replay validation was performed against LOBSTER AAPL historical datasets. Synthetic datasets reconstructed exactly. Historical datasets exhibited expected discrepancies due to orders that were already resting in the book before the start of the replay window. Investigation of unmatched deletion and execution events confirmed that many referenced order IDs had no corresponding submission event within the available message stream.