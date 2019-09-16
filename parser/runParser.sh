python myParser.py ebay_data/items-*.json
sort items.dat | uniq > items2.dat
sort users.dat | uniq > users2.dat
sort bids.dat | uniq > bids2.dat
sort categories.dat | uniq > categories2.dat
mv -f items2.dat ./items.dat
mv -f users2.dat ./users.dat
mv -f bids2.dat ./bids.dat
mv -f categories2.dat ./categories.dat
