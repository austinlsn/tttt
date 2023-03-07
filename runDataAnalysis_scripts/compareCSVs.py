import csv

def compareCSVs(file1, file2):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        csv1 = csv.reader(f1)
        csv2 = csv.reader(f2)

        header = next(csv1)  # skip header row

        with open('/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt//runDataAnalysis_scripts/output_compareCSVs.csv', 'w', newline='') as output_file:
            writer = csv.writer(output_file)
            writer.writerow(header)

            for row1 in csv1:
                match = False # initialize match bool for comparing extra/fewer events in one CSV than the other.
                f2.seek(0) # reset the file pointer to the beginning of file2 for each iteration of the csv1 loop
                for row2 in csv2:
                    if row1[0] == row2[0]:  # compare only the first value
                        diff = [float(r1) - float(r2) for r1, r2 in zip(row1[1:], row2[1:])]
                        writer.writerow(row1[:1] + diff)
                        match = True
                        break; # move to the next row in csv1
                if not match: # after the csv2 loop, if no matching event number in csv2, print the event number
                    print(row1[0])
                        

# Make sure to put the larger csv file first, if applicable.
inFile1 = '/home/users/yash/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.csv'
inFile2 = '/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.csv'
inFile22 = '/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/saved_outputs/DY_2l_M_50_5files/DY_2l_M_50.csv' 
compareCSVs(inFile1, inFile2)
