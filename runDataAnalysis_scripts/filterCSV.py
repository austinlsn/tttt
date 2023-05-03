# script to filter through output csv and show important values

# Columns of output csv:
# Event, has OS, nGenMatched leptons, nFlips, nTightCharge of leading lepton, nTightCharge of trailing lepton, mother of leading lepton's genmatch, mother of trailing lepton's genmatch
import pandas as pd


# Print the results
print('Number of events with ...')

# Load the CSV file into a DataFrame
df = pd.read_csv('/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/output_csvMaker.csv')


# Filter the DataFrame based on OS or SS events
OSfilter_df = df[df[' has OS'] == 1]
print(f'OS events: {len(OSfilter_df)}')
SSfilter_df = df[df[' has OS'] == 0]
print(f'SS events: {len(SSfilter_df)}')


OS_noFlips_df = df[(df[' has OS'] == 1) & (df[' nFake leptons'] == 0) & ((df[' nFlips'] == 0) | (df[' nFlips'] == 2))]
print(f'OS events w/ no fakes, not from SSgen: {len(OS_noFlips_df)}')
SS_noFlips_df = df[(df[' has OS'] == 0) & (df[' nFake leptons'] == 0) & (df[' nFlips'] == 1)]
print(f'SS events w/ no fakes, not from SSgen: {len(SS_noFlips_df)}')


print('\n')
# Eta overflow filters:
OS_etaoverflowfilter_df = df[(df[' has OS'] == 1) & (df[' nFake leptons'] == 0) & ((df[' nFlips'] == 0) | (df[' nFlips'] == 2)) & ((abs(df[' leading eta']) == 2.5) | (abs(df[' trailing eta']) == 2.5))]
print(f'OS events w/ |eta| = 2.5, no fakes, not from SSgen: {len(OS_etaoverflowfilter_df)}')
SS_etaoverflowfilter_df = df[(df[' has OS'] == 0) & (df[' nFake leptons'] == 0) & (df[' nFlips'] == 1) & ((abs(df[' leading eta']) == 2.5) | (abs(df[' trailing eta']) == 2.5))]
print(f'SS events w/ |eta| = 2.5, no fakes, not from SSgen: {len(SS_etaoverflowfilter_df)}')


print('\n')
# Filter by number of genmatches in SS events
SS_2GMfilter_df = df[(df[' nGenMatched leptons'] == 2) & (df[' has OS'] == 0)]
print(f'SS events w/ 2 genmatches: {len(SS_2GMfilter_df)}')

SS_1or0GMfilter_df = df[(df[' nGenMatched leptons'] < 2) & (df[' has OS'] == 0)]
print(f'SS events w/ <2 genmatches: {len(SS_1or0GMfilter_df)}')


print('\n')
# SS genevents filters:
SS_0flips_df    = df[(df[' nFlips'] == 0) & (df[' has OS'] == 0) & (df[' nGenMatched leptons'] == 2)]
print(f'SS events w/ 0 flips & 2 genmatches: {len(SS_0flips_df)}')

OS_1GMFlipsfilter_df = df[(df[' nFlips'] == 1) & (df[' has OS'] == 1) & (df[' nGenMatched leptons'] == 2)] # SS gen
print(f'OS events w/ 1 flip  & 2 genmatches: {len(OS_1GMFlipsfilter_df)}')


print('\n')
OS_2GMFlipsfilter_df = df[(df[' nFlips'] == 2) & (df[' has OS'] == 1) & (df[' nGenMatched leptons'] == 2)]
print(f'OS events w/ 2 flips & 2 genmatches: {len(OS_2GMFlipsfilter_df)}')

print(f'Gen SS events: {len(SS_0flips_df) + len(OS_1GMFlipsfilter_df)}')
