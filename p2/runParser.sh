python myParser.py ebay_data/items-*.json
sort items.dat | uniq > items.dat
sort users.dat | uniq > users.dat
sort bids.dat | uniq > bids.dat
