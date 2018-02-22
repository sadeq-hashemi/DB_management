
"""
FILE: skeleton_parser.py
------------------
Author: Firas Abuzaid (fabuzaid@stanford.edu)
Author: Perth Charernwattanagul (puch@stanford.edu)
Modified: 04/21/2014

Skeleton parser for CS564 programming project 1. Has useful imports and
functions for parsing, including:

1) Directory handling -- the parser takes a list of eBay json files
and opens each file inside of a loop. You just need to fill in the rest.
2) Dollar value conversions -- the json files store dollar value amounts in
a string like $3,453.23 -- we provide a function to convert it to a string
like XXXXX.xx.
3) Date/time conversions -- the json files store dates/ times in the form
Mon-DD-YY HH:MM:SS -- we wrote a function (transformDttm) that converts to the
for YYYY-MM-DD HH:MM:SS, which will sort chronologically in SQL.

Your job is to implement the parseJson function, which is invoked on each file by
the main function. We create the initial Python dictionary object of items for
you; the rest is up to you!
Happy parsing!
"""

import sys
from json import loads
from re import sub

columnSeparator = "|"
allitems = []
allbids = []
allusers = set()
allcategories = []
# Dictionary of months used for date transformation
MONTHS = {'Jan':'01','Feb':'02','Mar':'03','Apr':'04','May':'05','Jun':'06',\
        'Jul':'07','Aug':'08','Sep':'09','Oct':'10','Nov':'11','Dec':'12'}

"""
Returns true if a file ends in .json
"""
def isJson(f):
    return len(f) > 5 and f[-5:] == '.json'

"""
Converts month to a number, e.g. 'Dec' to '12'
"""
def transformMonth(mon):
    if mon in MONTHS:
        return MONTHS[mon]
    else:
        return mon

"""
Transforms a timestamp from Mon-DD-YY HH:MM:SS to YYYY-MM-DD HH:MM:SS
"""
def transformDttm(dttm):
    dttm = dttm.strip().split(' ')
    dt = dttm[0].split('-')
    date = '20' + dt[2] + '-'
    date += transformMonth(dt[0]) + '-' + dt[1]
    return date + ' ' + dttm[1]

"""
Transform a dollar value amount from a string like $3,453.23 to XXXXX.xx
"""

def transformDollar(money):
    if money == None or len(money) == 0:
        return money
    return sub(r'[^\d.]', '', money)

"""
Transform strings that my have single or double quotation into unescape format
"""
def transformString(str):
    temp = str.replace('"', '""')
    temp = "'"+ temp + "'"
    return temp
"""
Adds a string containing a value for all columns to the items list
ItemID | Name | Categories | Currently | Buy_Price | First_Bid | 
Number_of_Bids | Started | Ends | Description | UserID (seller)
"""
def getItem(item):
    #Adds ItemID to the string followed by a column separator    
    itm = str(item['ItemID']) + columnSeparator
        
    #Adds name to the string followed by a column separator 
    itm += transformString(item['Name']) + columnSeparator #adds ID and Name 

    #----------------------------    
    for i in item['Category']: #for every available category, adds to string separated by a comma
      #tmp = i.replace('"','""')
      #tmp = i.replace("'", "''")
      cat = transformString(i) + columnSeparator + item['ItemID']
      allcategories.append(cat)

    #Adds currently after having removed the $ sign to the string followed by a column separator     
    itm += transformDollar(item['Currently']) + columnSeparator

    #Adds Buy Price to the string followed by a column separator 
    if 'Buy_Price' in item : 
      itm += transformDollar(item['Buy_Price']) + columnSeparator
    else: 
      #Add null if there is no buy price
      itm += 'NULL' + columnSeparator
    
    #Adds the first bid and the number of bids to the string, each followed by a column separator after removing the $ sign
    itm = itm + transformDollar(item['First_Bid']) + columnSeparator + item['Number_of_Bids'] + columnSeparator
    #Adds the started time and the end time in the correct format to the string, each followed by a column separator 
    itm += transformDttm(item['Started']) + columnSeparator + transformDttm(item['Ends']) + columnSeparator

    #Check if there is a Description
    if item['Description'] is None :
      #Adds null to the string if there isn't any description followed by a column separator  
      itm += 'NULL' + columnSeparator
    else : 
      #Adds a description with the correct string format
      itm += transformString(item['Description']) + columnSeparator
    #Adds the UserID to the string
    itm += item['Seller']['UserID']
    return itm

"""
Gets all user info and adds as string in a list
all users have a UserID, Location, Country, and Rating
UserID | Rating | Location | Country 
"""
def getUser(item):
    usr = item['Seller']['UserID'] + columnSeparator + item['Seller']['Rating'] \
         + columnSeparator + transformString(item['Location']) + columnSeparator + item['Country']
    return usr

"""
Collects all info relating to bids and bidders. Bidder info will be added as user
and all details of the bid will go in a seperate list that links with the user
itemID | userID | time | Amount
"""
def getBid(item):
  if item['Bids'] is None :
    return 1
  for bid in item['Bids']:
    usr = bid['Bid']['Bidder']['UserID'] + columnSeparator + bid['Bid']['Bidder']['Rating'] + columnSeparator
    if 'Location' in bid['Bid']['Bidder']: 
      usr += transformString(bid['Bid']['Bidder']['Location']) + columnSeparator
    else: 
      usr += 'NULL' + columnSeparator
    if 'Country' in bid['Bid']['Bidder']: 
      usr += bid['Bid']['Bidder']['Country']
    else:
      usr += 'NULL'


    if usr not in allusers:  
      allusers.add(usr)
    bid = item['ItemID']+ columnSeparator + bid['Bid']['Bidder']['UserID'] +  columnSeparator +\
     transformDttm(bid['Bid']['Time']) +  columnSeparator + transformDollar(bid['Bid']['Amount']) 
    allbids.append(bid)
  return 
"""
Parses a single json file. Currently, there's a loop that iterates over each
item in the data set. Your job is to extend this functionality to create all
of the necessary SQL tables for your database.
"""
def parseJson(json_file):
    with open(json_file, 'r') as f:
        items = loads(f.read())['Items'] # creates a Python dictionary of Items for the supplied json file
        for item in items:
            """
            TODO: traverse the items dictionary to extract information from the
            given `json_file' and generate the necessary .dat files to generate
            the SQL tables based on your relation design
            """
            allitems.append(getItem(item))
            usr = getUser(item)
            if usr not in allusers:
              allusers.add(usr)
            getBid(item)
            pass
    f.close()


"""
checks the lists for all references to exist
"""
def checkData():
  err = 0
  userset = set()
  itemset = set()
  bidset = set()
  for usr in allusers:
    a = usr.split('|')
    if a[0] in userset:
      err+= 1
      print 'duplicate user found'
    else: 
      userset.add(a[0])

  for itm in allitems:
    a = itm.split('|')
    ID = a[0]
    seller = a[9]
    if seller not in userset:
      err+=1 
      print 'item seller not found'
    if ID in itemset:
      err+=1
      print 'duplicate item found'
    else: 
      itemset.add(ID)
 
  for bid in allbids:
     a = bid.split('|')
     item = a[0] 
     bidder= a[1]
     time = a[2]
     all = item + bidder + time
     if all in bidset: 
       print("duplicate bid")
     else: 
       bidset.add(all)
     if item not in itemset:
       err+=1
       print 'bid item not in items'
     if bidder not in userset:
       err +=1 
       print 'bidder ID not in users'

  for cat in allcategories:
     a = cat.split('|')
     item = a[1]
     if item not in itemset:
       print ('item reference in category does not exist in items')
"""
writes the collected lists of items, users, bids and categories to items.dat, users.dat, and bids.dat, categories.dat
"""
def writeData():
   #Write items to items.dat
    with open('items.dat', 'w') as f:
      for item in allitems: 
        f.write(item)
        f.write('\n')
        #print item
      f.close()

    #Sort the users
    sorted(allusers)
    #Write users to users.dat
    with open('users.dat', 'w') as f:
      for usr in allusers:
        f.write(usr)
        f.write('\n')
        #print usr
      f.close()
    
    #Write the bids to bids.dat
    with open('bids.dat', 'w') as f:
      for bid in allbids:
        f.write(bid)
        f.write('\n')
        #print bid
      f.close()
    
    #Write the categories to categories.dat
    with open('categories.dat', 'w') as f: 
      for cat in allcategories: 
        f.write(cat)
        f.write('\n')
      f.close()

"""
Loops through each json files provided on the command line and passes each file
to the parser
"""
def main(argv):
    if len(argv) < 2:
        print >> sys.stderr, 'Usage: python skeleton_json_parser.py <path to json files>'
        sys.exit(1)
    # loops over all .json files in the argument
    for f in argv[1:]:
        if isJson(f):
            parseJson(f)
            print "Success parsing " + f
    #Function called to write the four datas: items.dat, users.dat, bids.dat and categories.dat
    writeData()
    checkData()
if __name__ == '__main__':
    main(sys.argv)
