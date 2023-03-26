# script to filter through output csv and show important values

# Columns of output csv:
# Event, has OS, nGenMatched leptons, nFlips, nTightCharge of leading lepton, nTightCharge of trailing lepton, mother of leading lepton's genmatch, mother of trailing lepton's genmatch


import pandas as pd

# Load the CSV file into a DataFrame
df = pd.read_csv('/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/output_csvMaker.csv')

# Filter the DataFrame based on OS or SS events
OSfilter_df = df[df[' has OS'] == 1]
SSfilter_df = df[df[' has OS'] == 0]
# Filter by number of genmatches in SS events
SS_2GMfilter_df = df[(df[' nGenMatched leptons'] == 2) & (df[' has OS'] == 0)]
SS_1GMfilter_df = df[(df[' nGenMatched leptons'] == 1) & (df[' has OS'] == 0)]
SS_0GMfilter_df = df[(df[' nGenMatched leptons'] == 0) & (df[' has OS'] == 0)]


# Get the number of rows in the filtered DataFrame
num_OSevents = len(OSfilter_df)
num_SSevents = len(SSfilter_df)
num_SS_2GMevents = len(SS_2GMfilter_df)
num_SS_1GMevents = len(SS_1GMfilter_df)
num_SS_0GMevents = len(SS_0GMfilter_df)

# Print the results
print('Number of events with ...')
print(f'OS events: {num_OSevents}')
print(f'SS events: {num_SSevents}')
print(f'SS events w/ 2 genmatches: {num_SS_2GMevents}')
print(f'SS events w/ 1 genmatches: {num_SS_1GMevents}')
print(f'SS events w/ 0 genmatches: {num_SS_0GMevents}')
