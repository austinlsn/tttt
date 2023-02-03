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
                csv2 = csv.reader(f2)  # reset csv2 to the beginning
                for row2 in csv2:
                    if row1[0] == row2[0]:  # compare only the first value
                        diff = [float(r1) - float(r2) for r1, r2 in zip(row1[1:], row2[1:])]
                        writer.writerow(row1[:1] + diff)
                        break; # move to the next row in csv1


compareCSVs('/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/austinTest/2018/TTTT.csv'
            , '/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/austinTest/2018/TTTTYash.csv')
