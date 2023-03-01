import csv

def filter_by_nFlips(inFile, writer, filter_value): 
    nFlips_column = 7  # column which contains nFlips
    hasOS_column  = 3   # column which contains has_OS
    header = []  # initialize an empty header
    with open(inFile, 'r') as f_in:
        reader = csv.reader(f_in)
        next(reader)
        for i, row in enumerate(reader):
            if i == 0:  # header row
                header = []
                # add new column to header for this filter
                header.append('OS events with flips')
                writer.writerow(header)
            else:
                if (int(row[nFlips_column]) >= filter_value 
                    and int(row[hasOS_column]) == 1): # filter
                    # add filtered value to the new column
                    writer.writerow([row[0], int(row[nFlips_column])])


def general_filter(inFile, writer, filter_name, filter_column, filter_value): 
    header = []  # initialize an empty header
    with open(inFile, 'r') as f_in:
        reader = csv.reader(f_in)
        next(reader)
        for i, row in enumerate(reader):
            if i == 0:  # header row
                # add new column to header for this filter
                header.append(filter_name)
                writer.writerow(header)
            else:
                if (int(row[filter_column]) == filter_value):
                    # add filtered value to the new column
                    writer.writerow([row[0]])


def filterCSV():
    input_filename = '/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.csv'   
    input_filename_yash = '/home/users/yash/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.csv'
    output_filename = '/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/output_filterCSV.csv' 

    with open(output_filename, 'w', newline='') as f_out:
        writer = csv.writer(f_out)
        #filter_by_nFlips(input_filename, writer, 1) # OS w/ flips
        general_filter(input_filename, writer, 'SS events', 3, 0)
        #general_filter(input_filename_yash, writer, 'SS events Yash', 5, 1)

    # # Transpose entire CSV--possibly will break down w/ large dataset
    # with open(output_filename, 'r') as outFile:
    #     reader = csv.reader(outFile)
    #     output_data = [row for row in reader]
        
    #     transposed_data = list(map(list, zip(*output_data)))

    # with open(output_filename, 'w', newline='') as outFile:
    #     writer = csv.writer(outFile)
    #     writer.writerows(transposed_data)


filterCSV()
