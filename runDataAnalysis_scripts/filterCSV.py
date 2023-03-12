import csv

def filter_by_nFlips(inFile, writer, filter_value): 
    nGM_column     = 6    # column ... nGenMatched leptons
    nFlips_column  = 7    # column which contains nFlips
    hasOS_column   = 3    # column which contains has_OS
    header         = []   # initialize an empty header
    filtered_count = 0    # init for counting number of filtered items
    with open(inFile, 'r') as f_in:
        reader = csv.reader(f_in)
        for i, row in enumerate(reader):
            if i == 0:          # header row
                header = []
                header.append('OS events with flips, number of genmatched leptons')
                writer.writerow(header)
            else:               # filter
                if (int(row[nFlips_column]) >= filter_value 
                    and int(row[hasOS_column]) == 1): 
                    writer.writerow([row[0], row[nGM_column]])
                    filtered_count += 1

        writer.writerow(['total number of OS events with ' + str(filter_value) + ' flips: ' + str(filtered_count)])
        writer.writerow("")

def general_filter(inFile, writer, filter_name, filter_column, filter_value): 
    header         = []   # initialize an empty header
    filtered_count = 0    # init for counting number of filtered items
    with open(inFile, 'r') as f_in:
        reader = csv.reader(f_in)
        #next(reader)
        for i, row in enumerate(reader):
            if i == 0:          # header row
                header.append(filter_name)
                writer.writerow(header)
            else:               # filter
                if (int(row[filter_column]) == filter_value): 
                    writer.writerow([row[0]])
                    filtered_count += 1
        writer.writerow(['total number of ' + str(filter_name) + ': ' + str(filtered_count)])
        writer.writerow('')


def filterCSV():
    input_filename = '/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.csv'   
    input_filename_yash = '/home/users/yash/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.csv'
    #input_filename = '/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/saved_outputs/DY_2l_M_50_10files/DY_2l_M_50.csv'
    output_filename = '/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/output_filterCSV.csv' 

    with open(output_filename, 'w', newline='') as f_out:
        writer = csv.writer(f_out)
        filter_by_nFlips(input_filename, writer, 2) # OS w/ 2 flips
        filter_by_nFlips(input_filename, writer, 1) # OS w/ 1 flip
        general_filter(input_filename, writer, 'SS events', 3, 0)
        general_filter(input_filename, writer, 'OS events', 3, 1)
        general_filter(input_filename, writer, '0 Gen match', 6, 0)
        general_filter(input_filename, writer, '1 Gen match', 6, 1)
        #general_filter(input_filename, writer, '2 Gen match', 6, 2)
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
